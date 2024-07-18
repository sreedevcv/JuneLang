#include "Interpreter.hpp"

#include <map>

#include "Callable.hpp"
#include "ErrorHandler.hpp"
#include "NativeFunctions.hpp"

jl::Interpreter::Interpreter(Arena& arena, std::string& file_name, int64_t internal_arena_size)
    : m_arena(arena)
    , m_file_name(file_name)
    , m_internal_arena(internal_arena_size)
{
    m_env = m_internal_arena.allocate<Environment>(m_file_name);
    m_global_env = m_env;

    auto to_int_native_func = m_arena.allocate<ToIntNativeFunction>();
    auto to_str_native_func = m_arena.allocate<ToStrNativeFunction>();
    auto get_len_native_func = m_arena.allocate<GetLenNativeFunction>();
    auto append_native_func = m_arena.allocate<AppendNativeFunction>();
    auto remove_last_native_func = m_arena.allocate<RemoveLastNativeFunction>();
    auto clear_list_native_func = m_arena.allocate<ClearListNativeFunction>();
    m_global_env->define(to_int_native_func->m_name, static_cast<Callable*>(to_int_native_func));
    m_global_env->define(to_str_native_func->m_name, static_cast<Callable*>(to_str_native_func));
    m_global_env->define(get_len_native_func->m_name, static_cast<Callable*>(get_len_native_func));
    m_global_env->define(append_native_func->m_name, static_cast<Callable*>(append_native_func));
    m_global_env->define(remove_last_native_func->m_name, static_cast<Callable*>(remove_last_native_func));
    m_global_env->define(clear_list_native_func->m_name, static_cast<Callable*>(clear_list_native_func));
}

void jl::Interpreter::interpret(Expr* expr, Value* value)
{
    try {
        evaluate(expr);
    } catch (const char* exc) {
        ErrorHandler::m_stream << ErrorHandler::get_error_count() << " Error[s] occured" << std::endl;
    }
}

void jl::Interpreter::interpret(std::vector<Stmt*>& statements)
{
    try {
        for (auto stmt : statements) {
            stmt->accept(*this);
        }
    } catch (const char* exc) {
        ErrorHandler::m_stream << ErrorHandler::get_error_count() << " Error[s] ocuured" << std::endl;
    }
}

void jl::Interpreter::resolve(Expr* expr, int depth)
{
    m_locals[expr] = depth;
}

jl::Value jl::Interpreter::evaluate(Expr* expr)
{
    return std::any_cast<Value>(expr->accept(*this));
}

bool jl::Interpreter::is_truthy(Value& value)
{
    if (is_null(value) || (is_bool(value) && std::get<bool>(value) == false)) {
        return false;
    }
    return true;
}

template <typename Op>
jl::Value jl::Interpreter::do_arith_operation(Value& left, Value& right, Op op, int line)
{
    if (!is_number(left) || !is_number(right)) {
        ErrorHandler::error(m_file_name, "interpreting", "binary expression", line, "Left and right operands must be a number", 0);
        throw "runtime-error";
    }

    if (is_float(left) || is_float(right)) {
        double a = is_float(left) ? std::get<double>(left) : std::get<int>(left);
        double b = is_float(right) ? std::get<double>(right) : std::get<int>(right);
        left = op(a, b);
        return left;
    } else {
        int a = std::get<int>(left);
        int b = std::get<int>(right);
        left = op(a, b);
        return left;
    }
}

jl::Value jl::Interpreter::append_strings(Value& left, Value& right)
{
    std::string left_str = std::get<std::string>(left);
    std::string& right_str = std::get<std::string>(right);
    left_str.append(right_str);

    // Return appended strings
    return left_str;
}

void jl::Interpreter::execute_block(std::vector<Stmt*>& statements, Environment* new_env)
{
    Environment* previous = m_env;
    bool exception_ocurred = false;

    try {
        m_env = new_env;
        Value value;
        for (auto stmt : statements) {
            stmt->accept(*this);
        }

    } catch (Value value) {
        // This happens during a function return
        // Just rethrow the value so that FunctionCallable::call can handle it
        exception_ocurred = true;
        m_env = previous;
        throw;
    } catch (const char* msg) {
        exception_ocurred = true;
        m_env = previous;
    }

    if (!exception_ocurred) {
        m_env = previous;
    }
}

bool jl::Interpreter::is_equal(Value& left, Value& right)
{
    if (left.index() != right.index()) {
        return false;
    }
    return left == right;
}

jl::Value& jl::Interpreter::look_up_variable(Token& name, Expr* expr)
{
    if (m_locals.contains(expr)) {
        return m_env->get_at(name, m_locals[expr]);
    } else {
        return m_global_env->get(name);
    }
}

std::string jl::Interpreter::stringify(Value& value)
{
    if (is_null(value)) {
        return "null";
    } else if (is_bool(value)) {
        return std::get<bool>(value) ? "true" : "false";
    } else if (is_int(value)) {
        return std::to_string(std::get<int>(value));
    } else if (is_float(value)) {
        return std::to_string(std::get<double>(value));
    } else if (is_string(value)) {
        return std::get<std::string>(value);
    } else if (is_instance(value)) {
        return std::get<Instance*>(value)->to_string();
    } else if (is_callable(value)) {
        return std::get<Callable*>(value)->to_string();
    } else if (is_jlist(value)) {
        std::string list = "[";
        for (auto expr : *std::get<std::vector<Expr*>*>(value)) {
            if (dynamic_cast<Literal*>(expr)) {
                list.append(stringify(static_cast<Literal*>(expr)->m_value));
            } else {
                list.append("`expr`");
            }
            list.append(", ");
        }
        list.append("]");
        return list;
    }

    return "`null`";
}

// --------------------------------------------------------------------------------
// -------------------------------Expressions--------------------------------------
// --------------------------------------------------------------------------------

std::any jl::Interpreter::visit_assign_expr(Assign* expr)
{
    Value value = evaluate(expr->m_expr);

    if (m_locals.contains(expr)) {
        m_env->assign_at(expr->m_token, value, m_locals[expr]);
    } else {
        m_global_env->assign(expr->m_token, value);
    }

    return value;
}

std::any jl::Interpreter::visit_binary_expr(Binary* expr)
{
    Value left_value = evaluate(expr->m_left);
    Value right_value = evaluate(expr->m_right);
    int line = expr->m_oper->get_line();

    switch (expr->m_oper->get_tokentype()) {
    case Token::MINUS:
        return do_arith_operation(left_value, right_value, std::minus<>{}, line);
    case Token::STAR:
        return do_arith_operation(left_value, right_value, std::multiplies<>{}, line);
    case Token::SLASH:
        return do_arith_operation(left_value, right_value, std::divides<>{}, line);
    case Token::PLUS:
        if (is_string(left_value) && is_string(right_value)) {
            return append_strings(left_value, right_value);
        }
        return do_arith_operation(left_value, right_value, std::plus<>{}, line);
    case Token::GREATER:
        return do_arith_operation(left_value, right_value, std::greater<>{}, line);
    case Token::LESS:
        return do_arith_operation(left_value, right_value, std::less<>{}, line);
    case Token::GREATER_EQUAL:
        return do_arith_operation(left_value, right_value, std::greater_equal<>{}, line);
    case Token::LESS_EQUAL:
        return do_arith_operation(left_value, right_value, std::less_equal<>{}, line);
    case Token::PERCENT:
        if (!is_int(left_value) || !is_int(right_value)) {
            ErrorHandler::error(m_file_name, "interpreting", "binary expression", line, "Left and right operands must be a int to use `%`", 0);
            throw "runtime-error";
        }
        return Value(std::get<int>(left_value) % std::get<int>(right_value));
    case Token::EQUAL_EQUAL:
        return Value(is_equal(left_value, right_value));
    case Token::BANG_EQUAL:
        return Value(!is_equal(left_value, right_value));
    default:
        std::cout << "Unimplemented Operator in visit_binary_expr()\n";
        throw "runtime-error";
    }
}

std::any jl::Interpreter::visit_grouping_expr(Grouping* expr)
{
    return evaluate(expr->m_expr);
}

std::any jl::Interpreter::visit_unary_expr(Unary* expr)
{
    Value right_value = evaluate(expr->m_expr);

    switch (expr->m_oper->get_tokentype()) {
    case Token::MINUS:
        if (is_number(right_value)) {
            if (is_int(right_value)) {
                return Value(-1 * std::get<int>(right_value));
            } else {
                return Value(-1.0 * std::get<double>(right_value));
            }
        } else {
            ErrorHandler::error(m_file_name, "interpreting", "unary expression", expr->m_oper->get_line(), "Operand must be a number", 0);
            throw "runtime-error";
        }
        break;
    case Token::BANG:
        return Value(!is_truthy(right_value));
        break;
    default:
        break;
    }

    return Value(JNullType {});
}

std::any jl::Interpreter::visit_literal_expr(Literal* expr)
{
    return expr->m_value;
}

std::any jl::Interpreter::visit_variable_expr(Variable* expr)
{
    return look_up_variable(expr->m_name, expr);
}

std::any jl::Interpreter::visit_logical_expr(Logical* expr)
{
    // Value value;
    Value left = evaluate(expr->m_left);
    bool truth = is_truthy(left);

    if (expr->m_oper.get_tokentype() == Token::OR) {
        if (truth) {
            // return left since first OR is truthy
            return left;
        }
    } else {
        if (!truth) {
            // return left since first AND is falsey
            return left;
        }
    }

    return evaluate(expr->m_right);
}

std::any jl::Interpreter::visit_call_expr(Call* expr)
{
    Value value = evaluate(expr->m_callee);

    std::vector<Value> arguments(expr->m_arguments.size());

    for (int i = 0; i < expr->m_arguments.size(); i++) {
        arguments[i] = evaluate(expr->m_arguments[i]);
    }

    if (!is_callable(value)) {
        ErrorHandler::error(m_file_name, "interpreting", "function call", expr->m_paren.get_line(), "Only a function or class is callable", 0);
        throw "exception";
    }

    Callable* function = std::get<Callable*>(value);
    if (arguments.size() != function->arity()) {
        ErrorHandler::error(m_file_name, "interpreting", "function call", expr->m_paren.get_line(), "Arity of function call and its declararion do not match", 0);
        throw "exception";
    }

    Value return_value = function->call(this, arguments);
    return return_value;
}

std::any jl::Interpreter::visit_get_expr(Get* expr)
{
    Value value = evaluate(expr->m_object);

    if (is_instance(value)) {
        Value field = std::get<Instance*>(value)->get(expr->m_name);
        return field;
    } else {
        ErrorHandler::error(m_file_name, "interpreting", "get expression", expr->m_name.get_line(), "Attempted to get fields from a non-instance value", 0);
        throw "runtime-exception";
    }
}

std::any jl::Interpreter::visit_set_expr(Set* expr)
{
    Value value = evaluate(expr->m_object);

    if (!is_instance(value)) {
        ErrorHandler::error(m_file_name, "interpreting", "set expression", expr->m_name.get_line(), "Attempted to set fields to a non-instance value", 0);
        throw "runtime-exception";
    }

    Value setting_value = std::any_cast<Value>(evaluate(expr->m_value));
    std::get<Instance*>(value)->set(expr->m_name, setting_value);
    return setting_value;
}

std::any jl::Interpreter::visit_this_expr(This* expr)
{
    return look_up_variable(expr->m_keyword, expr);
}

std::any jl::Interpreter::visit_super_expr(Super* expr)
{
    int distance = m_locals[expr];
    ClassCallable* super_class = static_cast<ClassCallable*>(std::get<Callable*>(m_env->get_at(Token::global_super_lexeme, distance)));
    Value instance_value = m_env->get_at(Token::global_this_lexeme, distance - 1);
    Instance* instance = std::get<Instance*>(instance_value);
    FunctionCallable* method = super_class->find_method(expr->m_method.get_lexeme());

    if (method == nullptr) {
        ErrorHandler::error(m_file_name, "interpreting", "super keyword", expr->m_keyword.get_line(), "Udefined property called on super", 0);
        throw "runtime-exception";
    }

    return Value(static_cast<Callable*>(method->bind(instance)));
}

std::any jl::Interpreter::visit_jlist_expr(JList* expr)
{
    // Evaluate all the elements in jlist
    for (auto& item : expr->m_items) {
        if (dynamic_cast<Literal*>(item)) {
            continue; // No need to evaluate in case it is already a Literal
        }
        Value value = evaluate(item);
        item = m_internal_arena.allocate<Literal>(value);
    }

    // Create a new array so that a new JList is created everytime the node is interpreted
    // Otherwise JLists created as members of classes will always point to the same one
    // See::examples/EList.jun
    std::vector<Expr*>* items_copy = m_internal_arena.allocate<std::vector<Expr*>>(expr->m_items);
    return Value(items_copy);
}

std::any jl::Interpreter::visit_index_get_expr(IndexGet* expr)
{
    Value list_value = evaluate(expr->m_jlist);

    if (is_jlist(list_value)) {
        auto jlist = std::move(std::get<std::vector<Expr*>*>(list_value));
        Value index_value = evaluate(expr->m_index_expr);

        if (is_int(index_value)) {
            int index = std::get<int>(index_value);
            Value result = evaluate(jlist->at(index));
            return result;
        } else {
            ErrorHandler::error(m_file_name, "interpreting", "get index expression", expr->m_closing_bracket.get_line(), "Attempted to get index using a non-int value", 0);
            throw "runtime-exception";
        }
    } else {
        ErrorHandler::error(m_file_name, "interpreting", "get index expression", expr->m_closing_bracket.get_line(), "Attempted to get index from a value that is not a list", 0);
        throw "runtime-exception";
    }
}

std::any jl::Interpreter::visit_index_set_expr(IndexSet* expr)
{
    Value list_value = evaluate(expr->m_jlist);

    if (!is_jlist(list_value)) {
        ErrorHandler::error(m_file_name, "interpreting", "set index expression", expr->m_closing_bracket.get_line(), "Attempted to set index for a value that is not a list", 0);
        throw "runtime-exception";
    }

    Value index_value = evaluate(expr->m_index_expr);

    if (!is_int(index_value)) {
        ErrorHandler::error(m_file_name, "interpreting", "set index expression", expr->m_closing_bracket.get_line(), "Attempted to set index using a non-int value", 0);
        throw "runtime-exception";
    }

    auto jlist = std::get<std::vector<Expr*>*>(list_value);
    int index = std::get<int>(index_value);

    Value overwriting_value = evaluate(expr->m_value_expr);
    jlist->at(index) = m_internal_arena.allocate<Literal>(overwriting_value);
    return overwriting_value;
}

// --------------------------------------------------------------------------------
// -------------------------------Statements---------------------------------------
// --------------------------------------------------------------------------------

std::any jl::Interpreter::visit_print_stmt(PrintStmt* stmt)
{
    Value value = evaluate(stmt->m_expr);
    ErrorHandler::m_stream << stringify(value) << std::endl;
    return Value(JNullType {});
}

std::any jl::Interpreter::visit_expr_stmt(ExprStmt* stmt)
{
    evaluate(stmt->m_expr);
    return Value(JNullType {});
}

std::any jl::Interpreter::visit_var_stmt(VarStmt* stmt)
{
    Value value = JNullType {};
    if (stmt->m_initializer != nullptr) {
        value = evaluate(stmt->m_initializer);
    }

    m_env->define(stmt->m_name.get_lexeme(), value);
    return Value(JNullType {});
}

std::any jl::Interpreter::visit_block_stmt(BlockStmt* stmt)
{
    Environment* new_env = m_internal_arena.allocate<Environment>(m_env);
    execute_block(stmt->m_statements, new_env);
    return Value(JNullType {});
}

std::any jl::Interpreter::visit_empty_stmt(EmptyStmt* stmt)
{
    return Value(JNullType {});
}

std::any jl::Interpreter::visit_if_stmt(IfStmt* stmt)
{
    Value value = evaluate(stmt->m_condition);
    if (is_truthy(value)) {
        stmt->m_then_stmt->accept(*this);
    } else if (stmt->m_else_stmt != nullptr) {
        stmt->m_else_stmt->accept(*this);
    }

    return Value(JNullType {});
}

std::any jl::Interpreter::visit_while_stmt(WhileStmt* stmt)
{
    Value value = evaluate(stmt->m_condition);
    while (is_truthy(value)) {
        stmt->m_body->accept(*this);
        value = evaluate(stmt->m_condition);
    }

    return Value(JNullType {});
}

std::any jl::Interpreter::visit_func_stmt(FuncStmt* stmt)
{
    FunctionCallable* function = m_arena.allocate<FunctionCallable>(m_internal_arena, stmt, m_env, false);
    m_env->define(stmt->m_name.get_lexeme(), static_cast<Callable*>(function));

    return Value(JNullType {});
}

std::any jl::Interpreter::visit_return_stmt(ReturnStmt* stmt)
{
    Value value(JNullType {});
    if (stmt->m_expr != nullptr) {
        value = evaluate(stmt->m_expr);
    }

    throw value;
}

std::any jl::Interpreter::visit_class_stmt(ClassStmt* stmt)
{
    Value super_class = static_cast<Callable*>(nullptr);
    if (stmt->m_super_class != nullptr) {
        super_class = evaluate(stmt->m_super_class);
        if (!(is_callable(super_class) && dynamic_cast<ClassCallable*>(std::get<Callable*>(super_class)))) {
            ErrorHandler::error(m_file_name, "interpreting", "class definition", stmt->m_name.get_line(), "Super class must be a class", 0);
            throw "runtime-exception";
        }
    }

    m_env->define(stmt->m_name.get_lexeme(), static_cast<Callable*>(nullptr));

    if (stmt->m_super_class != nullptr) {
        m_env = m_internal_arena.allocate<Environment>(m_env);
        m_env->define(Token::global_super_lexeme, super_class);
    }

    std::map<std::string, FunctionCallable*> methods;
    for (FuncStmt* method : stmt->m_methods) {
        FunctionCallable* func_callable = m_arena.allocate<FunctionCallable>(m_internal_arena, method, m_env, method->m_name.get_lexeme() == "init");
        methods[method->m_name.get_lexeme()] = func_callable;
    }

    ClassCallable* class_callable = m_arena.allocate<ClassCallable>(stmt->m_name.get_lexeme(), static_cast<ClassCallable*>(std::get<Callable*>(super_class)), methods);

    if (stmt->m_super_class != nullptr) {
        m_env = m_env->m_enclosing;
    }

    m_env->assign(stmt->m_name, (static_cast<Callable*>(class_callable)));

    return Value(JNullType {});
}

std::any jl::Interpreter::visit_for_each_stmt(ForEachStmt* stmt)
{
    m_env = m_internal_arena.allocate<Environment>(m_env); // Create a new env for decalring looping variable
    stmt->m_var_declaration->accept(*this);
    Value value = evaluate(stmt->m_list_expr);

    if (!is_jlist(value)) {
        ErrorHandler::error(m_file_name, "interpreting", "for each", stmt->m_var_declaration->m_name.get_line(), "For each loops need a list to iterate", 0);
        throw "runtime-exception";
    }

    for (Expr* item : *std::get<std::vector<Expr*>*>(value)) {
        Value list_value = evaluate(item);
        m_env->assign(stmt->m_var_declaration->m_name, list_value);

        stmt->m_body->accept(*this);
    }

    m_env = m_env->m_enclosing;

    return Value(JNullType {});
}
