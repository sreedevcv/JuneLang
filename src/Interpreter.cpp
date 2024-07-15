#include "Interpreter.hpp"

#include <map>

#include "Callable.hpp"
#include "ErrorHandler.hpp"

jl::Interpreter::Interpreter(Arena& arena, std::string& file_name, int64_t internal_arena_size)
    : m_arena(arena)
    , m_file_name(file_name)
    , m_internal_arena(internal_arena_size)
{
    m_env = m_internal_arena.allocate<Environment>(m_file_name);
    m_global_env = m_env;

    ToIntNativeFunction* to_int_native_func = m_arena.allocate<ToIntNativeFunction>();
    m_global_env->define(to_int_native_func->m_name, static_cast<Callable*>(to_int_native_func));
}

jl::Interpreter::~Interpreter()
{
}

void jl::Interpreter::interpret(Expr* expr, Value* value)
{
    try {
        Value context;
        evaluate(expr, &context);
    } catch (const char* exc) {
        ErrorHandler::m_stream << ErrorHandler::get_error_count() << " Error[s] occured" << std::endl;
    }
}

void jl::Interpreter::interpret(std::vector<Stmt*>& statements)
{
    try {
        Value value;
        for (auto stmt : statements) {
            stmt->accept(*this, &value);
        }
    } catch (const char* exc) {
        ErrorHandler::m_stream << ErrorHandler::get_error_count() << " Error[s] ocuured" << std::endl;
    }
}

void jl::Interpreter::resolve(Expr* expr, int depth)
{
    m_locals[expr] = depth;
}

void jl::Interpreter::evaluate(Expr* expr, void* context)
{
    expr->accept(*this, context);
}

bool jl::Interpreter::is_truthy(Value* value)
{
    if (is_null(*value) || (is_bool(*value) && std::get<bool>(*value) == false)) {
        return false;
    }
    return true;
}

template <typename Op>
void jl::Interpreter::do_arith_operation(Value& left, Value& right, void* context, Op op)
{
    Value* value_context = static_cast<Value*>(context);
    if (is_float(left) || is_float(right)) {
        double a = is_float(left) ? std::get<double>(left) : std::get<int>(left);
        double b = is_float(right) ? std::get<double>(right) : std::get<int>(right);
        left = op(a, b);
        *value_context = left;
    } else {
        int a = std::get<int>(left);
        int b = std::get<int>(right);
        left = op(a, b);
        *value_context = left;
    }
}

void jl::Interpreter::append_strings(Value& left, Value& right, void* context)
{
    std::string left_str = std::get<std::string>(left);
    std::string& right_str = std::get<std::string>(right);
    left_str.append(right_str);

    // Return appended strings
    *static_cast<Value*>(context) = left_str;
}

void jl::Interpreter::execute_block(std::vector<Stmt*>& statements, Environment* new_env)
{
    Environment* previous = m_env;
    bool exception_ocurred = false;

    try {
        m_env = new_env;
        Value value;
        for (auto stmt : statements) {
            stmt->accept(*this, &value);
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
    } else {
        return std::get<Callable*>(value)->to_string();
    }
}

// --------------------------------------------------------------------------------
// -------------------------------Expressions--------------------------------------
// --------------------------------------------------------------------------------

void jl::Interpreter::visit_assign_expr(Assign* expr, void* context)
{
    evaluate(expr->m_expr, context);

    Value value = *static_cast<Value*>(context);
    if (m_locals.contains(expr)) {
        m_env->assign_at(expr->m_token, value, m_locals[expr]);
    } else {
        m_global_env->assign(expr->m_token, value);
    }
}

void jl::Interpreter::visit_binary_expr(Binary* expr, void* context)
{

    evaluate(expr->m_left, context);
    Value left = *static_cast<Value*>(context);
    evaluate(expr->m_right, context);
    Value right = *static_cast<Value*>(context);

    switch (expr->m_oper->get_tokentype()) {
    case Token::MINUS:
        if (is_number(left) && is_number(right)) {
            do_arith_operation(left, right, context, std::minus<>());
        } else {
            ErrorHandler::error(m_file_name, "interpreting", "binary expression", expr->m_oper->get_line(), "Left and right operands must be a number", 0);
            throw "runtime-error";
        }
        break;
    case Token::STAR:
        if (is_number(left) && is_number(right)) {
            do_arith_operation(left, right, context, std::multiplies<>());
        } else {
            ErrorHandler::error(m_file_name, "interpreting", "binary expression", expr->m_oper->get_line(), "Left and right operands must be a number", 0);
            throw "runtime-error";
        }
        break;
    case Token::SLASH:
        if (is_number(left) && is_number(right)) {
            do_arith_operation(left, right, context, std::divides<>());
        } else {
            ErrorHandler::error(m_file_name, "interpreting", "binary expression", expr->m_oper->get_line(), "Left and right operands must be a number", 0);
            throw "runtime-error";
        }
        break;
    case Token::PLUS:
        if (is_number(left) && is_number(right)) {
            do_arith_operation(left, right, context, std::plus<>());
        } else if (is_string(left) && is_string(right)) {
            append_strings(left, right, context);
        } else {
            ErrorHandler::error(m_file_name, "interpreting", "binary expression", expr->m_oper->get_line(), "Left and right operands must be a number", 0);
            throw "runtime-error";
        }
        break;
    case Token::GREATER:
        if (is_number(left) && is_number(right)) {
            do_arith_operation(left, right, context, std::greater<>());
        } else {
            ErrorHandler::error(m_file_name, "interpreting", "binary expression", expr->m_oper->get_line(), "Left and right operands must be a number", 0);
            throw "runtime-error";
        }
        break;
    case Token::LESS:
        if (is_number(left) && is_number(right)) {
            do_arith_operation(left, right, context, std::less<>());
        } else {
            ErrorHandler::error(m_file_name, "interpreting", "binary expression", expr->m_oper->get_line(), "Left and right operands must be a number", 0);
            throw "runtime-error";
        }
        break;
    case Token::GREATER_EQUAL:
        if (is_number(left) && is_number(right)) {
            do_arith_operation(left, right, context, std::greater_equal<>());
        } else {
            ErrorHandler::error(m_file_name, "interpreting", "binary expression", expr->m_oper->get_line(), "Left and right operands must be a number", 0);
            throw "runtime-error";
        }
        break;
    case Token::LESS_EQUAL:
        if (is_number(left) && is_number(right)) {
            do_arith_operation(left, right, context, std::less_equal<>());
        } else {
            ErrorHandler::error(m_file_name, "interpreting", "binary expression", expr->m_oper->get_line(), "Left and right operands must be a number", 0);
            throw "runtime-error";
        }
        break;
    case Token::EQUAL_EQUAL:
        *static_cast<Value*>(context) = is_equal(left, right);
        break;
    case Token::BANG_EQUAL:
        *static_cast<Value*>(context) = !is_equal(left, right);
        break;
    default:
        break;
    }
}

void jl::Interpreter::visit_grouping_expr(Grouping* expr, void* context)
{
    evaluate(expr->m_expr, context);
}

void jl::Interpreter::visit_unary_expr(Unary* expr, void* context)
{
    evaluate(expr->m_expr, context);
    Value* right = static_cast<Value*>(context);

    switch (expr->m_oper->get_tokentype()) {
    case Token::MINUS:
        if (is_number(*right)) {
            if (is_int(*right)) {
                *right = -1 * std::get<int>(*right);
            } else {
                *right = -1.0 * std::get<double>(*right);
            }
            context = right;
        } else {
            ErrorHandler::error(m_file_name, "interpreting", "unary expression", expr->m_oper->get_line(), "Operand must be a number", 0);
            throw "runtime-error";
        }
        break;
    case Token::BANG:
        *right = !is_truthy(static_cast<Value*>(right));
        context = right;
        break;
    default:
        break;
    }
}

void jl::Interpreter::visit_literal_expr(Literal* expr, void* context)
{
    *static_cast<Value*>(context) = *expr->m_value;
}

// Returns the token value as the context
void jl::Interpreter::visit_variable_expr(Variable* expr, void* context)
{
    *static_cast<Value*>(context) = look_up_variable(expr->m_name, expr);
}

void jl::Interpreter::visit_logical_expr(Logical* expr, void* context)
{
    // Value value;
    evaluate(expr->m_left, context);
    bool truth = is_truthy(static_cast<Value*>(context));

    if (expr->m_oper.get_tokentype() == Token::OR) {
        if (truth) {
            // return whatever is in context since first OR is truthy
            return;
        }
    } else {
        if (!truth) {
            // return whatever is in context since first AND is falsey
            return;
        }
    }

    evaluate(expr->m_right, context);
    // Return whatever is evaluated inside right expr`
}

void jl::Interpreter::visit_call_expr(Call* expr, void* context)
{
    evaluate(expr->m_callee, context);
    Value* value = static_cast<Value*>(context);

    std::vector<Value> arguments(expr->m_arguments.size());

    for (int i = 0; i < expr->m_arguments.size(); i++) {
        evaluate(expr->m_arguments[i], &arguments[i]);
    }

    if (!is_callable(*value)) {
        ErrorHandler::error(m_file_name, "interpreting", "function call", expr->m_paren.get_line(), "Only a function or class is callable", 0);
        throw "exception";
    }

    Callable* function = std::get<Callable*>(*value);
    if (arguments.size() != function->arity()) {
        ErrorHandler::error(m_file_name, "interpreting", "function call", expr->m_paren.get_line(), "Arity of function call and its declararion do not match", 0);
        throw "exception";
    }

    Value return_value = function->call(this, arguments);
    *static_cast<Value*>(context) = return_value;
}

void jl::Interpreter::visit_get_expr(Get* expr, void* context)
{
    Value* value = static_cast<Value*>(context);
    evaluate(expr->m_object, value);

    if (is_instance(*value)) {
        *static_cast<Value*>(context) = std::get<Instance*>(*value)->get(expr->m_name);
    } else {
        ErrorHandler::error(m_file_name, "interpreting", "get expression", expr->m_name.get_line(), "Attempted to get fields from a non-instance value", 0);
        throw "runtime-exception";
    }
}

void jl::Interpreter::visit_set_expr(Set* expr, void* context)
{
    Value* object = static_cast<Value*>(context);
    evaluate(expr->m_object, object);

    if (!is_instance(*object)) {
        ErrorHandler::error(m_file_name, "interpreting", "set expression", expr->m_name.get_line(), "Attempted to set fields to a non-instance value", 0);
        throw "runtime-exception";
    }

    Value value;
    evaluate(expr->m_value, &value);
    std::get<Instance*>(*object)->set(expr->m_name, value);
    *static_cast<Value*>(context) = value;
}

void jl::Interpreter::visit_this_expr(This* expr, void* context)
{
    *static_cast<Value*>(context) = look_up_variable(expr->m_keyword, expr);
}

void jl::Interpreter::visit_super_expr(Super* expr, void* context)
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

    *static_cast<Value*>(context) = static_cast<Callable*>(method->bind(instance));
}

void jl::Interpreter::visit_jlist_expr(JList* expr, void* context)
{
    // Evaluate all the elements in jlist
    for (auto& item: expr->m_items) {
        if (dynamic_cast<Literal*>(item)) {
            continue;   // No need to evaluate in case it is already a Literal
        }
        Value* value = m_internal_arena.allocate<Value>();
        evaluate(item, value);
        item = m_internal_arena.allocate<Literal>(value);
    }

    Value* value = m_internal_arena.allocate<Value>();
    *value = &expr->m_items;
    *static_cast<Value*>(context) = *value;
}

void jl::Interpreter::visit_index_get_expr(IndexGet* expr, void* context)
{
    Value* list_value = static_cast<Value*>(context);
    evaluate(expr->m_jlist, list_value);

    if (is_jlist(*list_value)) {
        auto jlist = std::move(std::get<std::vector<Expr*>*>(*list_value));
        Value index_value;
        evaluate(expr->m_index_expr, &index_value);

        if (is_int(index_value)) {
            Value result;
            int index = std::get<int>(index_value);
            evaluate(jlist->at(index), &result);
            *static_cast<Value*>(context) = result;
        } else {
            ErrorHandler::error(m_file_name, "interpreting", "get index expression", expr->m_closing_bracket.get_line(), "Attempted to get index using a non-int value", 0);
            throw "runtime-exception";
        }
    } else {
        ErrorHandler::error(m_file_name, "interpreting", "get index expression", expr->m_closing_bracket.get_line(), "Attempted to get index from a value that is not a list", 0);
        throw "runtime-exception";
    }
}

void jl::Interpreter::visit_index_set_expr(IndexSet* expr, void* context)
{
    Value* list_value = static_cast<Value*>(context);
    evaluate(expr->m_jlist, list_value);

    if (!is_jlist(*list_value)) {
        ErrorHandler::error(m_file_name, "interpreting", "set index expression", expr->m_closing_bracket.get_line(), "Attempted to set index for a value that is not a list", 0);
        throw "runtime-exception";
    }

    Value index_value;
    evaluate(expr->m_index_expr, &index_value);

    if (!is_int(index_value)) {
        ErrorHandler::error(m_file_name, "interpreting", "set index expression", expr->m_closing_bracket.get_line(), "Attempted to set index using a non-int value", 0);
        throw "runtime-exception";
    }

    auto jlist = std::get<std::vector<Expr*>*>(*list_value);
    int index = std::get<int>(index_value);

    // Do we need to evaluate whatever is currently existing at the index??
    // Value set_result;
    // evaluate(jlist[index], &set_result);

    Value* overwriting_value = m_internal_arena.allocate<Value>();
    evaluate(expr->m_value_expr, overwriting_value);
    jlist->at(index) = m_internal_arena.allocate<Literal>(overwriting_value);        
}

void* jl::Interpreter::get_expr_context()
{
    return nullptr;
}

// --------------------------------------------------------------------------------
// -------------------------------Statements---------------------------------------
// --------------------------------------------------------------------------------

void jl::Interpreter::visit_print_stmt(PrintStmt* stmt, void* context)
{
    evaluate(stmt->m_expr, context);
    Value* value = static_cast<Value*>(context);
    ErrorHandler::m_stream << stringify(*value) << std::endl;
}

void jl::Interpreter::visit_expr_stmt(ExprStmt* stmt, void* context)
{
    // Evaluate and discard the context value
    evaluate(stmt->m_expr, context);
}

void jl::Interpreter::visit_var_stmt(VarStmt* stmt, void* context)
{
    // Set value as null
    // Value value = '\0';
    Value* value = static_cast<Value*>(context);
    if (stmt->m_initializer != nullptr) {
        evaluate(stmt->m_initializer, value);
    }

    m_env->define(stmt->m_name.get_lexeme(), *value);
}

void jl::Interpreter::visit_block_stmt(BlockStmt* stmt, void* context)
{
    Environment* new_env = m_internal_arena.allocate<Environment>(m_env);
    execute_block(stmt->m_statements, new_env);
}

void jl::Interpreter::visit_empty_stmt(EmptyStmt* stmt, void* context)
{
}

void jl::Interpreter::visit_if_stmt(IfStmt* stmt, void* context)
{
    Value* value = static_cast<Value*>(context);
    evaluate(stmt->m_condition, value);
    if (is_truthy(value)) {
        stmt->m_then_stmt->accept(*this, context);
    } else if (stmt->m_else_stmt != nullptr) {
        stmt->m_else_stmt->accept(*this, context);
    }
}

void jl::Interpreter::visit_while_stmt(WhileStmt* stmt, void* context)
{
    Value value;
    evaluate(stmt->m_condition, &value);
    while (is_truthy(&value)) {
        stmt->m_body->accept(*this, context);
        evaluate(stmt->m_condition, &value);
    }

    // make context null
    value = '\0';
    *static_cast<Value*>(context) = value;
}

void jl::Interpreter::visit_func_stmt(FuncStmt* stmt, void* context)
{
    FunctionCallable* function = m_arena.allocate<FunctionCallable>(m_internal_arena, stmt, m_env, false);
    m_env->define(stmt->m_name.get_lexeme(), static_cast<Callable*>(function));
    *static_cast<Value*>(context) = '\0';
}

void jl::Interpreter::visit_return_stmt(ReturnStmt* stmt, void* context)
{
    Value value = '\0';
    if (stmt->m_expr != nullptr) {
        evaluate(stmt->m_expr, &value);
    }

    throw value;
}

void jl::Interpreter::visit_class_stmt(ClassStmt* stmt, void* context)
{
    Value super_class = static_cast<Callable*>(nullptr);
    if (stmt->m_super_class != nullptr) {
        evaluate(stmt->m_super_class, &super_class);
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
}

void* jl::Interpreter::get_stmt_context()
{
    return nullptr;
}
