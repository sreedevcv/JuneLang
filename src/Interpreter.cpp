#include "Interpreter.hpp"

#include "ErrorHandler.hpp"

void jl::Interpreter::interpret(Expr* expr, Token::Value* value)
{
    try {
        Token::Value context;
        evaluate(expr, &context);
        *value = context;
    } catch (const char* exc) {
        std::cout << ErrorHandler::get_error_count() << " Errors ocuured" << std::endl;
    }
}

void jl::Interpreter::interpret(std::vector<Stmt*>& statements)
{
    try {
        Token::Value value;
        for (auto stmt : statements) {
            stmt->accept(*this, &value);
        }
    } catch (const char* exc) {
        std::cout << ErrorHandler::get_error_count() << " Errors ocuured" << std::endl;
    }
}

void jl::Interpreter::visit_assign_expr(Assign* expr, void* context)
{
    evaluate(expr->m_expr, context);
    env.assign(expr->m_token, *static_cast<Token::Value*>(context));
}

void jl::Interpreter::visit_binary_expr(Binary* expr, void* context)
{
    evaluate(expr->m_left, context);
    Token::Value left = *static_cast<Token::Value*>(context);
    evaluate(expr->m_right, context);
    Token::Value right = *static_cast<Token::Value*>(context);

    switch (expr->m_oper->get_tokentype()) {
    case Token::MINUS:
        if (is_number(left) && is_number(right)) {
            do_arith_operation(left, right, context, std::minus<>());
        } else {
            ErrorHandler::error(file_name, expr->m_oper->get_line(), "operands must be a number");
            throw "runtime-error";
        }
        break;
    case Token::STAR:
        if (is_number(left) && is_number(right)) {
            do_arith_operation(left, right, context, std::multiplies<>());
        } else {
            ErrorHandler::error(file_name, expr->m_oper->get_line(), "operands must be a number");
            throw "runtime-error";
        }
        break;
    case Token::SLASH:
        if (is_number(left) && is_number(right)) {
            do_arith_operation(left, right, context, std::divides<>());
        } else {
            ErrorHandler::error(file_name, expr->m_oper->get_line(), "operands must be a number");
            throw "runtime-error";
        }
        break;
    case Token::PLUS:
        if (is_number(left) && is_number(right)) {
            do_arith_operation(left, right, context, std::plus<>());
        } else if (is_string(left) && is_string(right)) {
            append_strings(left, right, context);
        } else {
            ErrorHandler::error(file_name, expr->m_oper->get_line(), "operands must be a number");
            throw "runtime-error";
        }
        break;
    case Token::GREATER:
        if (is_number(left) && is_number(right)) {
            do_arith_operation(left, right, context, std::greater<>());
        } else {
            ErrorHandler::error(file_name, expr->m_oper->get_line(), "operands must be a number");
            throw "runtime-error";
        }
        break;
    case Token::LESS:
        if (is_number(left) && is_number(right)) {
            do_arith_operation(left, right, context, std::less<>());
        } else {
            ErrorHandler::error(file_name, expr->m_oper->get_line(), "operands must be a number");
            throw "runtime-error";
        }
        break;
    case Token::GREATER_EQUAL:
        if (is_number(left) && is_number(right)) {
            do_arith_operation(left, right, context, std::greater_equal<>());
        } else {
            ErrorHandler::error(file_name, expr->m_oper->get_line(), "operands must be a number");
            throw "runtime-error";
        }
        break;
    case Token::LESS_EQUAL:
        if (is_number(left) && is_number(right)) {
            do_arith_operation(left, right, context, std::less_equal<>());
        } else {
            ErrorHandler::error(file_name, expr->m_oper->get_line(), "operands must be a number");
            throw "runtime-error";
        }
        break;
    case Token::EQUAL_EQUAL:
        *static_cast<Token::Value*>(context) = is_equal(left, right);
        break;
    case Token::BANG_EQUAL:
        *static_cast<Token::Value*>(context) = !is_equal(left, right);
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
    Token::Value* right = static_cast<Token::Value*>(context);

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
            ErrorHandler::error(file_name, expr->m_oper->get_line(), "operand must be a number");
            throw "runtime-error";
        }
        break;
    case Token::BANG:
        *right = !is_truthy(static_cast<Token::Value*>(right));
        context = right;
        break;
    default:
        break;
    }
}

void jl::Interpreter::visit_literal_expr(Literal* expr, void* context)
{
    *static_cast<Token::Value*>(context) = *expr->m_value;
}

// Returns the token value as the context
void jl::Interpreter::visit_variable_expr(Variable* expr, void* context)
{
    *static_cast<Token::Value*>(context) = env.get(expr->m_name);
}

void* jl::Interpreter::get_expr_context()
{
    return nullptr;
}

void jl::Interpreter::visit_print_stmt(PrintStmt* stmt, void* context)
{
    evaluate(stmt->m_expr, context);
    Token::Value* value = static_cast<Token::Value*>(context);
    std::cout << stringify(*value);
}

void jl::Interpreter::visit_expr_stmt(ExprStmt* stmt, void* context)
{
    // Evaluate and discard the context value
    evaluate(stmt->m_expr, context);
}

void jl::Interpreter::visit_var_stmt(VarStmt* stmt, void* context)
{
    // Set value as null
    Token::Value value = '\0';
    if (stmt->m_initializer != nullptr) {
        evaluate(stmt->m_initializer, &value);
    }

    env.define(stmt->m_name.get_lexeme(), value);
}

void* jl::Interpreter::get_stmt_context()
{
    return nullptr;
}

void jl::Interpreter::evaluate(Expr* expr, void* context)
{
    expr->accept(*this, context);
}

bool jl::Interpreter::is_truthy(Token::Value* value)
{
    if (is_null(*value) || (is_bool(*value) && std::get<bool>(*value) == false)) {
        return false;
    }
    return true;
}

template <typename Op>
void jl::Interpreter::do_arith_operation(Token::Value& left, Token::Value& right, void* context, Op op)
{
    Token::Value* value_context = static_cast<Token::Value*>(context);
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

void jl::Interpreter::append_strings(Token::Value& left, Token::Value& right, void* context)
{
    std::string left_str = std::get<std::string>(left);
    std::string& right_str = std::get<std::string>(right);
    left_str.append(right_str);

    *static_cast<Token::Value*>(context) = left_str;
}

bool jl::Interpreter::is_equal(Token::Value& left, Token::Value& right)
{
    if (left.index() != right.index()) {
        return false;
    }
    return left == right;
}

std::string jl::Interpreter::stringify(Token::Value& value)
{
    if (is_null(value)) {
        return "null";
    } else if (is_bool(value)) {
        return std::get<bool>(value) ? "true" : "false";
    } else if (is_int(value)) {
        return std::to_string(std::get<int>(value));
    } else if (is_float(value)) {
        return std::to_string(std::get<double>(value));
    } else {
        return std::get<std::string>(value);
    }
}
