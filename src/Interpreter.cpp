#include "Interpreter.hpp"

#include <map>

#include "Callable.hpp"
#include "ErrorHandler.hpp"
#include "NativeFunctions.hpp"
#include "Value.hpp"

jl::Interpreter::Interpreter(Arena& arena, std::string& file_name, int64_t internal_arena_size)
    : m_arena(arena)
    , m_file_name(file_name)
    , m_internal_arena(internal_arena_size)
    , m_gc(m_global_env, m_env)
    , m_dummy_env(file_name)
{
    //m_env = m_gc.allocate<Environment>(m_file_name);
    m_env = &m_dummy_env;
    m_global_env = m_env;
    m_env_stack.push_back(m_global_env);

    auto to_int_native_func = m_arena.allocate<ToIntNativeFunction>();
    auto to_str_native_func = m_arena.allocate<ToStrNativeFunction>();
    auto get_len_native_func = m_arena.allocate<GetLenNativeFunction>();
    auto append_native_func = m_arena.allocate<AppendNativeFunction>();
    auto remove_last_native_func = m_arena.allocate<RemoveLastNativeFunction>();
    auto clear_list_native_func = m_arena.allocate<ClearListNativeFunction>();

    auto to_int_native_func_val = m_gc.allocate<JlCallable>(static_cast<Callable*>(to_int_native_func));
    auto to_str_native_func_val = m_gc.allocate<JlCallable>(static_cast<Callable*>(to_str_native_func));
    auto get_len_native_func_val = m_gc.allocate<JlCallable>(static_cast<Callable*>(get_len_native_func));
    auto append_native_func_val = m_gc.allocate<JlCallable>(static_cast<Callable*>(append_native_func));
    auto remove_last_native_func_val = m_gc.allocate<JlCallable>(static_cast<Callable*>(remove_last_native_func));
    auto clear_list_native_func_val = m_gc.allocate<JlCallable>(static_cast<Callable*>(clear_list_native_func));
    
    m_global_env->define(to_int_native_func->m_name, to_int_native_func_val);
    m_global_env->define(to_str_native_func->m_name, to_str_native_func_val);
    m_global_env->define(get_len_native_func->m_name, get_len_native_func_val);
    m_global_env->define(append_native_func->m_name, append_native_func_val);
    m_global_env->define(remove_last_native_func->m_name, remove_last_native_func_val);
    m_global_env->define(clear_list_native_func->m_name, clear_list_native_func_val);
}

jl::Interpreter::~Interpreter()
{
}

void jl::Interpreter::interpret(Expr* expr, JlValue* value)
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

jl::JlValue* jl::Interpreter::evaluate(Expr* expr)
{
    return std::any_cast<JlValue*>(expr->accept(*this));
}

bool jl::Interpreter::is_truthy(JlValue* value)
{
    if (is::_null(value) || (is::_bool(value) && jl::vget<bool>(value) == false)) {
        return false;
    }
    return true;
}

template <typename Op>
jl::JlValue* jl::Interpreter::do_arith_operation(JlValue* left, JlValue* right, Op op, int line)
{
    if (!is::_number(left) || !is::_number(right)) {
        ErrorHandler::error(m_file_name, "interpreting", "binary expression", line, "Left and right operands must be a number", 0);
        throw "runtime-error";
    }

    if (is::_float(left) || is::_float(right)) {
        double a = is::_float(left) ? jl::vget<double>(left) : jl::vget<int>(left);
        double b = is::_float(right) ? jl::vget<double>(right) : jl::vget<int>(right);
        if (is::_float(left)) {
            static_cast<JlFloat*>(left)->m_val = op(a, b);
        }
        else {
            static_cast<JlInt*>(left)->m_val = op(a, b);
        }
        return left;
    } else {
        int a = jl::vget<int>(left);
        int b = jl::vget<int>(right);
        static_cast<JlInt*>(left)->m_val = op(a, b);
        return left;
    }
}

jl::JlValue* jl::Interpreter::append_strings(JlValue* left, JlValue* right)
{
    std::string left_str = jl::vget<std::string>(left);
    std::string& right_str = jl::vget<std::string&>(right);
    left_str.append(right_str);

    // Return appended strings
    return m_gc.allocate<JlStr>(left_str);
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

    } catch (JlValue* value) {
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

bool jl::Interpreter::is_equal(JlValue* left, JlValue* right)
{
    if (is::_same(left, right)) {
        return false;
    }
    return is::_exact_same(left, right);
}

jl::JlValue* jl::Interpreter::look_up_variable(Token& name, Expr* expr)
{
    if (m_locals.contains(expr)) {
        return m_env->get_at(name, m_locals[expr]);
    } else {
        return m_global_env->get(name);
    }
}

std::string jl::Interpreter::stringify(JlValue* value)
{
    if (is::_null(value)) {
        return "null";
    } else if (is::_bool(value)) {
        return jl::vget<bool>(value) ? "true" : "false";
    } else if (is::_int(value)) {
        return std::to_string(jl::vget<int>(value));
    } else if (is::_float(value)) {
        return std::to_string(jl::vget<double>(value));
    } else if (is::_str(value)) {
        return jl::vget<std::string>(value);
    } else if (is::_obj(value)) {
        return jl::vget<Instance*>(value)->to_string();
    } else if (is::_callable(value)) {
        return jl::vget<Callable*>(value)->to_string();
    } else if (is::_list(value)) {
        std::string list = "[";
        for (auto expr : *jl::vget<std::vector<Expr*>*>(value)) {
            if (dynamic_cast<Literal*>(expr)) {
                list.append(stringify(static_cast<Literal*>(expr)->m_value));  //NOTE::Problem to pass address og no-gc allocated Jlvalue here??
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
    JlValue* value = evaluate(expr->m_expr);

    if (m_locals.contains(expr)) {
        m_env->assign_at(expr->m_token, value, m_locals[expr]);
    } else {
        m_global_env->assign(expr->m_token, value);
    }

    return value;
}

std::any jl::Interpreter::visit_binary_expr(Binary* expr)
{
    JlValue* left_value = evaluate(expr->m_left);
    JlValue* right_value = evaluate(expr->m_right);
    int line = expr->m_oper->get_line();

    switch (expr->m_oper->get_tokentype()) {
    case Token::MINUS:
        return do_arith_operation(left_value, right_value, std::minus<> {}, line);
    case Token::STAR:
        return do_arith_operation(left_value, right_value, std::multiplies<> {}, line);
    case Token::SLASH:
        return do_arith_operation(left_value, right_value, std::divides<> {}, line);
    case Token::PLUS:
        if (is::_str(left_value) && is::_str(right_value)) {
            return append_strings(left_value, right_value);
        }
        return do_arith_operation(left_value, right_value, std::plus<> {}, line);
    case Token::GREATER:
        return do_arith_operation(left_value, right_value, std::greater<> {}, line);
    case Token::LESS:
        return do_arith_operation(left_value, right_value, std::less<> {}, line);
    case Token::GREATER_EQUAL:
        return do_arith_operation(left_value, right_value, std::greater_equal<> {}, line);
    case Token::LESS_EQUAL:
        return do_arith_operation(left_value, right_value, std::less_equal<> {}, line);
    case Token::PERCENT:
        if (!is::_int(left_value) || !is::_int(right_value)) {
            ErrorHandler::error(m_file_name, "interpreting", "binary expression", line, "Left and right operands must be a int to use `%`", 0);
            throw "runtime-error";
        }
        return m_gc.allocate<JlInt>(jl::vget<int>(left_value) % jl::vget<int>(right_value));
    case Token::EQUAL_EQUAL:
        return m_gc.allocate<JlBool>(is_equal(left_value, right_value));
    case Token::BANG_EQUAL:
        return m_gc.allocate<JlBool>(!is_equal(left_value, right_value));
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
    JlValue* right_value = evaluate(expr->m_expr);

    switch (expr->m_oper->get_tokentype()) {
    case Token::MINUS:
        if (is::_number(right_value)) {
            if (is::_int(right_value)) {
                return m_gc.allocate<JlInt>(-1 * jl::vget<int>(right_value));
            } else {
                return m_gc.allocate<JlFloat>(-1.0 * jl::vget<double>(right_value));
            }
        } else {
            ErrorHandler::error(m_file_name, "interpreting", "unary expression", expr->m_oper->get_line(), "Operand must be a number", 0);
            throw "runtime-error";
        }
        break;
    case Token::BANG:
        return m_gc.allocate<JlBool>(!is_truthy(right_value));
        break;
    default:
        break;
    }

    return m_gc.allocate<JlNull>();
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
    // JlValue value;
    JlValue* left = evaluate(expr->m_left);
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
    JlValue* value = evaluate(expr->m_callee);

    std::vector<JlValue*> arguments(expr->m_arguments.size());

    for (int i = 0; i < expr->m_arguments.size(); i++) {
        arguments[i] = evaluate(expr->m_arguments[i]);
    }

    if (!is::_callable(value)) {
        ErrorHandler::error(m_file_name, "interpreting", "function call", expr->m_paren.get_line(), "Only a function or class is callable", 0);
        throw "exception";
    }

    Callable* function = jl::vget<Callable*>(value);
    if (arguments.size() != function->arity()) {
        ErrorHandler::error(m_file_name, "interpreting", "function call", expr->m_paren.get_line(), "Arity of function call and its declararion do not match", 0);
        throw "exception";
    }

    JlValue* return_value = function->call(this, arguments);
    return return_value;
}

std::any jl::Interpreter::visit_get_expr(Get* expr)
{
    JlValue* value = evaluate(expr->m_object);

    if (is::_obj(value)) {
        JlValue* field = jl::vget<Instance*>(value)->get(expr->m_name, this);
        return field;
    } else {
        ErrorHandler::error(m_file_name, "interpreting", "get expression", expr->m_name.get_line(), "Attempted to get fields from a non-instance value", 0);
        throw "runtime-exception";
    }
}

std::any jl::Interpreter::visit_set_expr(Set* expr)
{
    JlValue* value = evaluate(expr->m_object);

    if (!is::_obj(value)) {
        ErrorHandler::error(m_file_name, "interpreting", "set expression", expr->m_name.get_line(), "Attempted to set fields to a non-instance value", 0);
        throw "runtime-exception";
    }

    JlValue* setting_value = evaluate(expr->m_value);
    jl::vget<Instance*>(value)->set(expr->m_name, setting_value);
    return setting_value;
}

std::any jl::Interpreter::visit_this_expr(This* expr)
{
    return look_up_variable(expr->m_keyword, expr);
}

std::any jl::Interpreter::visit_super_expr(Super* expr)
{
    int distance = m_locals[expr];
    ClassCallable* super_class = static_cast<ClassCallable*>(jl::vget<Callable*>(m_env->get_at(Token::global_super_lexeme, distance)));
    JlValue* instance_value = m_env->get_at(Token::global_this_lexeme, distance - 1);
    Instance* instance = jl::vget<Instance*>(instance_value);
    FunctionCallable* method = super_class->find_method(expr->m_method.get_lexeme());

    if (method == nullptr) {
        ErrorHandler::error(m_file_name, "interpreting", "super keyword", expr->m_keyword.get_line(), "Udefined property called on super", 0);
        throw "runtime-exception";
    }

    return m_gc.allocate<JlCallable>(static_cast<Callable*>(method->bind(instance)));
}

std::any jl::Interpreter::visit_jlist_expr(JList* expr)
{
    // Evaluate all the elements in jlist
    for (auto& item : expr->m_items) {
        if (dynamic_cast<Literal*>(item)) {
            continue; // No need to evaluate in case it is already a Literal
        }
        JlValue* value = evaluate(item);
        item = m_gc.allocate<Literal>(value);
    }

    // Create a new array so that a new JList is created everytime the node is interpreted
    // Otherwise JLists created as members of classes will always point to the same one
    // See::examples/EList.jun
    // NOTE::This will leak!!!!
    std::vector<Expr*>* items_copy = m_internal_arena.allocate<std::vector<Expr*>>(expr->m_items);
    // std::vector<Expr*>* items_copy = m_gc.allocate<std::vector<Expr*>>(expr->m_items);
    return new JlList(items_copy); // NOTE::Also leaks!!!
}

std::any jl::Interpreter::visit_index_get_expr(IndexGet* expr)
{
    JlValue* list_value = evaluate(expr->m_jlist);

    if (is::_list(list_value)) {
        auto jlist = std::move(jl::vget<std::vector<Expr*>*>(list_value));
        JlValue* index_value = evaluate(expr->m_index_expr);

        if (is::_int(index_value)) {
            int index = jl::vget<int>(index_value);
            JlValue* result = evaluate(jlist->at(index));
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
    JlValue* list_value = evaluate(expr->m_jlist);

    if (!is::_list(list_value)) {
        ErrorHandler::error(m_file_name, "interpreting", "set index expression", expr->m_closing_bracket.get_line(), "Attempted to set index for a value that is not a list", 0);
        throw "runtime-exception";
    }

    JlValue* index_value = evaluate(expr->m_index_expr);

    if (!is::_int(index_value)) {
        ErrorHandler::error(m_file_name, "interpreting", "set index expression", expr->m_closing_bracket.get_line(), "Attempted to set index using a non-int value", 0);
        throw "runtime-exception";
    }

    auto jlist = jl::vget<std::vector<Expr*>*>(list_value);
    int index = jl::vget<int>(index_value);

    JlValue* overwriting_value = evaluate(expr->m_value_expr);
    jlist->at(index) = m_gc.allocate<Literal>(overwriting_value);
    return overwriting_value;
}

// --------------------------------------------------------------------------------
// -------------------------------Statements---------------------------------------
// --------------------------------------------------------------------------------

std::any jl::Interpreter::visit_print_stmt(PrintStmt* stmt)
{
    JlValue* value = evaluate(stmt->m_expr);
    ErrorHandler::m_stream << stringify(value) << std::endl;
    return m_gc.allocate<JlNull>();
}

std::any jl::Interpreter::visit_expr_stmt(ExprStmt* stmt)
{
    evaluate(stmt->m_expr);
    return m_gc.allocate<JlNull>();
}

std::any jl::Interpreter::visit_var_stmt(VarStmt* stmt)
{
    JlValue* value = m_gc.allocate<JlNull>();
    if (stmt->m_initializer != nullptr) {
        value = evaluate(stmt->m_initializer);
    }

    m_env->define(stmt->m_name.get_lexeme(), value);
    return m_gc.allocate<JlNull>();
}

std::any jl::Interpreter::visit_block_stmt(BlockStmt* stmt)
{
    Environment* new_env = m_gc.allocate<Environment>(m_env);
    execute_block(stmt->m_statements, new_env);
    return m_gc.allocate<JlNull>();
}

std::any jl::Interpreter::visit_empty_stmt(EmptyStmt* stmt)
{
    return m_gc.allocate<JlNull>();
}

std::any jl::Interpreter::visit_if_stmt(IfStmt* stmt)
{
    JlValue* value = evaluate(stmt->m_condition);
    if (is_truthy(value)) {
        stmt->m_then_stmt->accept(*this);
    }
    else if (stmt->m_else_stmt != nullptr) {
        stmt->m_else_stmt->accept(*this);
    }

    return m_gc.allocate<JlNull>();;
}

std::any jl::Interpreter::visit_while_stmt(WhileStmt* stmt)
{
    JlValue* value = evaluate(stmt->m_condition);
    while (is_truthy(value)) {
        // Exceptions thrown by breaks are handled here
        try {
            stmt->m_body->accept(*this);
        }
        catch (BreakThrow break_throw) {
            break;
        }
        value = evaluate(stmt->m_condition);
    }

    return m_gc.allocate<JlNull>();;
}

std::any jl::Interpreter::visit_func_stmt(FuncStmt* stmt)
{
    JlCallable* callable = m_gc.allocate<JlCallable>(static_cast<Callable*>(nullptr));
    m_env->define(stmt->m_name.get_lexeme(), callable);
    FunctionCallable* function = m_gc.allocate<FunctionCallable>(this, m_env, stmt, false);
    callable->m_val = function;

    return m_gc.allocate<JlNull>();
}

std::any jl::Interpreter::visit_return_stmt(ReturnStmt* stmt)
{
    JlValue* value = m_gc.allocate<JlNull>();
    if (stmt->m_expr != nullptr) {
        value = evaluate(stmt->m_expr);
    }

    throw value;
}

std::any jl::Interpreter::visit_class_stmt(ClassStmt* stmt)
{
    JlValue* super_class = m_gc.allocate<JlCallable>(static_cast<Callable*>(nullptr));
    if (stmt->m_super_class != nullptr) {
        super_class = evaluate(stmt->m_super_class);
        if (!(is::_callable(super_class) && dynamic_cast<ClassCallable*>(jl::vget<Callable*>(super_class)))) {
            ErrorHandler::error(m_file_name, "interpreting", "class definition", stmt->m_name.get_line(), "Super class must be a class", 0);
            throw "runtime-exception";
        }
    }

    m_env->define(stmt->m_name.get_lexeme(), m_gc.allocate<JlCallable>(static_cast<Callable*>(nullptr)));

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

    ClassCallable* class_callable = m_gc.allocate<ClassCallable>(stmt->m_name.get_lexeme(), static_cast<ClassCallable*>(jl::vget<Callable*>(super_class)), methods);

    if (stmt->m_super_class != nullptr) {
        m_env = m_env->m_enclosing;
        m_env_stack.pop_back();
    }

    m_env->assign(stmt->m_name, m_gc.allocate<JlCallable>(static_cast<Callable*>(class_callable)));

    return m_gc.allocate<JlNull>();
}

std::any jl::Interpreter::visit_for_each_stmt(ForEachStmt* stmt)
{
    m_env = m_gc.allocate<Environment>(m_env); // Create a new env for decalring looping variable
    m_env_stack.push_back(m_env);
    stmt->m_var_declaration->accept(*this);
    JlValue* value = evaluate(stmt->m_list_expr);

    if (!is::_list(value)) {
        ErrorHandler::error(m_file_name, "interpreting", "for each", stmt->m_var_declaration->m_name.get_line(), "For each loops need a list to iterate", 0);
        throw "runtime-exception";
    }

    for (Expr* item : *jl::vget<std::vector<Expr*>*>(value)) {
        JlValue* list_value = evaluate(item);
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

    return m_gc.allocate<JlNull>();
}

std::any jl::Interpreter::visit_break_stmt(BreakStmt* stmt)
{
    throw BreakThrow {};
}
