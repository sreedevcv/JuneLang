#include "Interpreter.hpp"

#include <map>

#include "Callable.hpp"
#include "ErrorHandler.hpp"
#include "NativeFunctions.hpp"
#include "Value.hpp"

jl::Interpreter::Interpreter(std::string& file_name)
    : m_file_name(file_name)
    , m_gc(m_global_env, m_env, m_env_stack)
    , m_dummy_env(file_name)
{
    m_env = &m_dummy_env;
    m_global_env = m_env;
    m_env_stack.push_back(m_global_env);

    auto to_int_native_func = new ToIntNativeFunction();
    auto to_str_native_func = new ToStrNativeFunction();
    auto get_len_native_func = new GetLenNativeFunction();
    auto append_native_func = new AppendNativeFunction();
    auto remove_last_native_func = new RemoveLastNativeFunction();
    auto clear_list_native_func = new ClearListNativeFunction();

    m_allocated_refs.push_back(to_int_native_func);
    m_allocated_refs.push_back(to_str_native_func);
    m_allocated_refs.push_back(get_len_native_func);
    m_allocated_refs.push_back(append_native_func);
    m_allocated_refs.push_back(remove_last_native_func);
    m_allocated_refs.push_back(clear_list_native_func);

    auto to_int_native_func_val = m_gc.allocate<Value>(static_cast<Callable*>(to_int_native_func));
    m_global_env->define(to_int_native_func->m_name, to_int_native_func_val);
    auto to_str_native_func_val = m_gc.allocate<Value>(static_cast<Callable*>(to_str_native_func));
    m_global_env->define(to_str_native_func->m_name, to_str_native_func_val);
    auto get_len_native_func_val = m_gc.allocate<Value>(static_cast<Callable*>(get_len_native_func));
    m_global_env->define(get_len_native_func->m_name, get_len_native_func_val);
    auto append_native_func_val = m_gc.allocate<Value>(static_cast<Callable*>(append_native_func));
    m_global_env->define(append_native_func->m_name, append_native_func_val);
    auto remove_last_native_func_val = m_gc.allocate<Value>(static_cast<Callable*>(remove_last_native_func));
    m_global_env->define(remove_last_native_func->m_name, remove_last_native_func_val);
    auto clear_list_native_func_val = m_gc.allocate<Value>(static_cast<Callable*>(clear_list_native_func));
    m_global_env->define(clear_list_native_func->m_name, clear_list_native_func_val);
}

jl::Interpreter::~Interpreter()
{
    for (auto ref : m_allocated_refs) {
        delete ref;
    }
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

jl::Value* jl::Interpreter::evaluate(Expr* expr)
{
    auto ret = expr->accept(*this);
    return std::any_cast<Value*>(ret);
}

bool jl::Interpreter::is_truthy(Value* value)
{
    if (is::_null(*value) || (is::_bool(*value) && std::get<bool>(value->get()) == false)) {
        return false;
    }
    return true;
}

template <typename Op>
jl::Value* jl::Interpreter::do_arith_operation(Value* left, Value* right, Op op, int line, bool is_logical)
{
    if (!is::_number(*left) || !is::_number(*right)) {
        ErrorHandler::error(m_file_name, "interpreting", "binary expression", line, "Left and right operands must be a number", 0);
        throw "runtime-error";
    }

    if (is::_float(*left) || is::_float(*right)) {
        double a = is::_float(*left) ? std::get<double>(left->get()) : std::get<int>(left->get());
        double b = is::_float(*right) ? std::get<double>(right->get()) : std::get<int>(right->get());

        if (is_logical) {
            return m_gc.allocate<Value>(op(a, b));
        } else {
            return m_gc.allocate<Value>(op(a, b));
        }
    } else {
        int a = std::get<int>(left->get());
        int b = std::get<int>(right->get());

        if (is_logical) {
            return m_gc.allocate<Value>(op(a, b));
        } else {
            return m_gc.allocate<Value>(op(a, b));
        }
    }
}

jl::Value* jl::Interpreter::append_strings(Value* left, Value* right)
{
    std::string left_str = std::get<std::string>(left->get());
    std::string& right_str = std::get<std::string>(right->get());
    left_str.append(right_str);

    // Return appended strings
    return m_gc.allocate<Value>(left_str);
}

void jl::Interpreter::execute_block(std::vector<Stmt*>& statements, Environment* new_env)
{
    Environment* previous = m_env;
    m_env_stack.push_back(new_env);
    bool exception_ocurred = false;

    try {
        m_env = new_env;
        for (auto stmt : statements) {
            stmt->accept(*this);
        }

    } catch (Value* value) {
        // This happens during a function return
        // Just rethrow the value so that FunctionCallable::call can handle it
        exception_ocurred = true;
        m_env = previous;
        m_env_stack.pop_back();
        throw;
    } catch (const char* msg) {
        // An error occurred
        exception_ocurred = true;
        m_env = previous;
        m_env_stack.pop_back();
    }

    if (!exception_ocurred) {
        m_env = previous;
        m_env_stack.pop_back();
    }
}

bool jl::Interpreter::is_equal(Value* left, Value* right)
{
    if (!is::_same(*left, *right)) {
        return false;
    }
    return is::_exact_same(*left, *right);
}

jl::Value* jl::Interpreter::look_up_variable(Token& name, Expr* expr)
{
    if (m_locals.contains(expr)) {
        return m_env->get_at(name, m_locals[expr]);
    } else {
        return m_global_env->get(name);
    }
}


// --------------------------------------------------------------------------------
// -------------------------------Expressions--------------------------------------
// --------------------------------------------------------------------------------

std::any jl::Interpreter::visit_assign_expr(Assign* expr)
{
    Value* value = evaluate(expr->m_expr);

    if (m_locals.contains(expr)) {
        m_env->assign_at(expr->m_token, value, m_locals[expr]);
    } else {
        m_global_env->assign(expr->m_token, value);
    }

    return value;
}

std::any jl::Interpreter::visit_binary_expr(Binary* expr)
{
    Value* left_value = evaluate(expr->m_left);
    Value* right_value = evaluate(expr->m_right);
    int line = expr->m_oper->get_line();

    switch (expr->m_oper->get_tokentype()) {
    case Token::MINUS:
        return do_arith_operation(left_value, right_value, std::minus<> {}, line);
    case Token::STAR:
        return do_arith_operation(left_value, right_value, std::multiplies<> {}, line);
    case Token::SLASH:
        return do_arith_operation(left_value, right_value, std::divides<> {}, line);
    case Token::PLUS:
        if (is::_str(*left_value) && is::_str(*right_value)) {
            return append_strings(left_value, right_value);
        }
        return do_arith_operation(left_value, right_value, std::plus<> {}, line);
    case Token::GREATER:
        return do_arith_operation(left_value, right_value, std::greater<> {}, line, true);
    case Token::LESS:
        return do_arith_operation(left_value, right_value, std::less<> {}, line, true);
    case Token::GREATER_EQUAL:
        return do_arith_operation(left_value, right_value, std::greater_equal<> {}, line, true);
    case Token::LESS_EQUAL:
        return do_arith_operation(left_value, right_value, std::less_equal<> {}, line, true);
    case Token::PERCENT:
        if (!is::_int(*left_value) || !is::_int(*right_value)) {
            ErrorHandler::error(m_file_name, "interpreting", "binary expression", line, "Left and right operands must be a int to use `%`", 0);
            throw "runtime-error";
        }
        return m_gc.allocate<Value>(std::get<int>(left_value->get()) % std::get<int>(right_value->get()));
    case Token::EQUAL_EQUAL:
        return m_gc.allocate<Value>(is_equal(left_value, right_value));
    case Token::BANG_EQUAL:
        return m_gc.allocate<Value>(!is_equal(left_value, right_value));
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
    Value* right_value = evaluate(expr->m_expr);

    switch (expr->m_oper->get_tokentype()) {
    case Token::MINUS:
        if (is::_number(*right_value)) {
            if (is::_int(*right_value)) {
                return m_gc.allocate<Value>(-1 * std::get<int>(right_value->get()));
            } else {
                return m_gc.allocate<Value>(-1.0 * std::get<double>(right_value->get()));
            }
        } else {
            ErrorHandler::error(m_file_name, "interpreting", "unary expression", expr->m_oper->get_line(), "Operand must be a number", 0);
            throw "runtime-error";
        }
        break;
    case Token::BANG:
        return m_gc.allocate<Value>(!is_truthy(right_value));
        break;
    default:
        break;
    }

    return m_gc.allocate<Value>();
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
    Value* left = evaluate(expr->m_left);
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
    Value* value = evaluate(expr->m_callee);

    std::vector<Value*> arguments(expr->m_arguments.size());

    for (int i = 0; i < expr->m_arguments.size(); i++) {
        arguments[i] = evaluate(expr->m_arguments[i]);
    }

    if (!is::_callable(*value)) {
        ErrorHandler::error(m_file_name, "interpreting", "function call", expr->m_paren.get_line(), "Only a function or class is callable", 0);
        throw "exception";
    }

    Callable* function = std::get<Callable*>(value->get());
    if (arguments.size() != function->arity()) {
        ErrorHandler::error(m_file_name, "interpreting", "function call", expr->m_paren.get_line(), "Arity of function call and its declararion do not match", 0);
        throw "exception";
    }

    Value* return_value = function->call(this, arguments);
    return return_value;
}

std::any jl::Interpreter::visit_get_expr(Get* expr)
{
    Value* value = evaluate(expr->m_object);

    if (is::_obj(*value)) {
        Value* field = std::get<Instance*>(value->get())->get(expr->m_name, this);
        return field;
    } else {
        ErrorHandler::error(m_file_name, "interpreting", "get expression", expr->m_name.get_line(), "Attempted to get fields from a non-instance value", 0);
        throw "runtime-exception";
    }
}

std::any jl::Interpreter::visit_set_expr(Set* expr)
{
    Value* value = evaluate(expr->m_object);

    if (!is::_obj(*value)) {
        ErrorHandler::error(m_file_name, "interpreting", "set expression", expr->m_name.get_line(), "Attempted to set fields to a non-instance value", 0);
        throw "runtime-exception";
    }

    Value* setting_value = evaluate(expr->m_value);
    std::get<Instance*>(value->get())->set(expr->m_name, setting_value);
    return setting_value;
}

std::any jl::Interpreter::visit_this_expr(This* expr)
{
    return look_up_variable(expr->m_keyword, expr);
}

std::any jl::Interpreter::visit_super_expr(Super* expr)
{
    int distance = m_locals[expr];
    auto& value = (m_env->get_at(Token::global_super_lexeme, distance))->get();
    ClassCallable* super_class = static_cast<ClassCallable*>(std::get<Callable*>(value));
    Value* instance_value = m_env->get_at(Token::global_this_lexeme, distance - 1);
    Instance* instance = std::get<Instance*>(instance_value->get());
    FunctionCallable* method = super_class->find_method(expr->m_method.get_lexeme());

    if (method == nullptr) {
        ErrorHandler::error(m_file_name, "interpreting", "super keyword", expr->m_keyword.get_line(), "Udefined property called on super", 0);
        throw "runtime-exception";
    }

    Value* callable = m_gc.allocate<Value>(static_cast<Callable*>(method->bind(instance)));
    return callable;
}

std::any jl::Interpreter::visit_jlist_expr(JList* expr)
{
    // Evaluate all the elements in jlist
    for (auto& item : expr->m_items) {
        // NOTE::Not sure precalculating is the right wau
        // if (dynamic_cast<Literal*>(item)) {
        //    continue; // No need to evaluate in case it is already a Literal
        //}
        Value* value = evaluate(item);
        item = m_gc.allocate<Literal>(value);
    }

    // Create a new array so that a new JList is created everytime the node is interpreted
    // Otherwise JLists created as members of classes will always point to the same one
    // See::examples/EList.jun
    // NOTE::This will leak!!!!
    // std::vector<Expr*>* items_copy = m_internal_arena.allocate<std::vector<Expr*>>(expr->m_items);
    Value* items_copy = m_gc.allocate<Value>(expr->m_items); // Copies the vecotr
    return items_copy; // NOTE::Also leaks!!!
}

std::any jl::Interpreter::visit_index_get_expr(IndexGet* expr)
{
    Value* list_value = evaluate(expr->m_jlist);

    if (is::_list(*list_value)) {
        auto& jlist = std::get<std::vector<Expr*>>(list_value->get());
        Value* index_value = evaluate(expr->m_index_expr);

        if (is::_int(*index_value)) {
            int index = std::get<int>(index_value->get());
            Value* result = evaluate(jlist.at(index));
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
    Value* list_value = evaluate(expr->m_jlist);

    if (!is::_list(*list_value)) {
        ErrorHandler::error(m_file_name, "interpreting", "set index expression", expr->m_closing_bracket.get_line(), "Attempted to set index for a value that is not a list", 0);
        throw "runtime-exception";
    }

    Value* index_value = evaluate(expr->m_index_expr);

    if (!is::_int(*index_value)) {
        ErrorHandler::error(m_file_name, "interpreting", "set index expression", expr->m_closing_bracket.get_line(), "Attempted to set index using a non-int value", 0);
        throw "runtime-exception";
    }

    auto& jlist = std::get<std::vector<Expr*>>(list_value->get());
    int index = std::get<int>(index_value->get());

    Value* overwriting_value = evaluate(expr->m_value_expr);
    jlist.at(index) = m_gc.allocate<Literal>(overwriting_value);
    return overwriting_value;
}

// --------------------------------------------------------------------------------
// -------------------------------Statements---------------------------------------
// --------------------------------------------------------------------------------

std::any jl::Interpreter::visit_print_stmt(PrintStmt* stmt)
{
    Value* value = evaluate(stmt->m_expr);
    ErrorHandler::m_stream << stringify(value) << std::endl;
    return m_gc.allocate<Value>(Value { Null {} });
}

std::any jl::Interpreter::visit_expr_stmt(ExprStmt* stmt)
{
    evaluate(stmt->m_expr);
    return m_gc.allocate<Value>(Null {});
}

std::any jl::Interpreter::visit_var_stmt(VarStmt* stmt)
{
    Value* value = m_gc.allocate<Value>(Null {});
    if (stmt->m_initializer != nullptr) {
        value = evaluate(stmt->m_initializer);
    }

    m_env->define(stmt->m_name.get_lexeme(), value);
    return m_gc.allocate<Value>(Value { Null {} });
}

std::any jl::Interpreter::visit_block_stmt(BlockStmt* stmt)
{
    Environment* new_env = m_gc.allocate<Environment>(m_env);
    execute_block(stmt->m_statements, new_env);
    return m_gc.allocate<Value>(Value { Null {} });
}

std::any jl::Interpreter::visit_empty_stmt(EmptyStmt* stmt)
{
    return m_gc.allocate<Value>(Value { Null {} });
}

std::any jl::Interpreter::visit_if_stmt(IfStmt* stmt)
{
    Value* value = evaluate(stmt->m_condition);
    if (is_truthy(value)) {
        stmt->m_then_stmt->accept(*this);
    } else if (stmt->m_else_stmt != nullptr) {
        stmt->m_else_stmt->accept(*this);
    }

    return m_gc.allocate<Value>(Value { Null {} });
}

std::any jl::Interpreter::visit_while_stmt(WhileStmt* stmt)
{
    Value* value = evaluate(stmt->m_condition);
    while (is_truthy(value)) {
        // Exceptions thrown by breaks are handled here
        try {
            stmt->m_body->accept(*this);
        } catch (BreakThrow break_throw) {
            break;
        }
        value = evaluate(stmt->m_condition);
    }

    return m_gc.allocate<Value>(Value { Null {} });
}

std::any jl::Interpreter::visit_func_stmt(FuncStmt* stmt)
{
    Value* callable = m_gc.allocate<Value>(static_cast<Callable*>(nullptr));
    m_env->define(stmt->m_name.get_lexeme(), callable);
    FunctionCallable* function = m_gc.allocate<FunctionCallable>(this, m_env, stmt, false);
    std::get<Callable*>(callable->get()) = function;

    return m_gc.allocate<Value>(Value { Null {} });
}

std::any jl::Interpreter::visit_return_stmt(ReturnStmt* stmt)
{
    Value* value = m_gc.allocate<Value>(Null{});
    if (stmt->m_expr != nullptr) {
        value = evaluate(stmt->m_expr);
    }

    throw value;
}

std::any jl::Interpreter::visit_class_stmt(ClassStmt* stmt)
{
    Value* super_class = m_gc.allocate<Value>(static_cast<Callable*>(nullptr));
    if (stmt->m_super_class != nullptr) {
        super_class = evaluate(stmt->m_super_class);
        if (!(is::_callable(*super_class) && dynamic_cast<ClassCallable*>(std::get<Callable*>(super_class->get())))) {
            ErrorHandler::error(m_file_name, "interpreting", "class definition", stmt->m_name.get_line(), "Super class must be a class", 0);
            throw "runtime-exception";
        }
    }

    m_env->define(stmt->m_name.get_lexeme(), m_gc.allocate<Value>(static_cast<Callable*>(nullptr)));

    if (stmt->m_super_class != nullptr) {
        m_env = m_gc.allocate<Environment>(m_env);
        m_env_stack.push_back(m_env);
        m_env->define(Token::global_super_lexeme, super_class);
    }

    std::map<std::string, FunctionCallable*> methods;
    for (FuncStmt* method : stmt->m_methods) {
        FunctionCallable* func_callable = m_gc.allocate<FunctionCallable>(this, m_env, method, method->m_name.get_lexeme() == "init");
        methods[method->m_name.get_lexeme()] = func_callable;
    }

    ClassCallable* class_callable = m_gc.allocate<ClassCallable>(stmt->m_name.get_lexeme(), static_cast<ClassCallable*>(std::get<Callable*>(super_class->get())), methods);

    if (stmt->m_super_class != nullptr) {
        m_env = m_env->m_enclosing;
        m_env_stack.pop_back();
    }

    m_env->assign(stmt->m_name, m_gc.allocate<Value>(static_cast<Callable*>(class_callable)));

    return m_gc.allocate<Value>(Value { Null {} });
}

std::any jl::Interpreter::visit_for_each_stmt(ForEachStmt* stmt)
{
    m_env = m_gc.allocate<Environment>(m_env); // Create a new env for decalring looping variable
    m_env_stack.push_back(m_env);
    stmt->m_var_declaration->accept(*this);
    Value* value = evaluate(stmt->m_list_expr);

    if (!is::_list(*value)) {
        ErrorHandler::error(m_file_name, "interpreting", "for each", stmt->m_var_declaration->m_name.get_line(), "For each loops need a list to iterate", 0);
        throw "runtime-exception";
    }

    for (Expr* item : std::get<std::vector<Expr*>>(value->get())) {
        Value* list_value = evaluate(item);
        m_env->assign(stmt->m_var_declaration->m_name, list_value);

        // Handling breaks
        try {
            stmt->m_body->accept(*this);
        } catch (BreakThrow break_throw) {
            // NOTE::Should i revert the env back???
            break;
        }
    }

    m_env = m_env->m_enclosing;
    m_env_stack.pop_back();

    return m_gc.allocate<Value>(Value { Null {} });
}

std::any jl::Interpreter::visit_break_stmt(BreakStmt* stmt)
{
    throw BreakThrow {};
}
