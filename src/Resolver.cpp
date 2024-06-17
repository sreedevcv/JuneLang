#include "Resolver.hpp"

#include "ErrorHandler.hpp"

jl::Resolver::Resolver(Interpreter& interpreter, std::string& file_name)
    : m_interpreter(interpreter)
    , m_file_name(file_name)
{
}

void jl::Resolver::resolve(std::vector<Stmt*>& statements)
{
    for (Stmt* stmt : statements) {
        resolve(stmt);
    }
}

void jl::Resolver::resolve(Stmt* statement)
{
    statement->accept(*this, nullptr);
}

void jl::Resolver::resolve(Expr* expression)
{
    expression->accept(*this, nullptr);
}

void jl::Resolver::resolve_local(Expr* expr, Token& name)
{
    for (int i = m_scopes.size() - 1; i >= 0; i--) {
        if (m_scopes[i].contains(name.get_lexeme())) {
            m_interpreter.resolve(expr, m_scopes.size() - i - 1);
            return;
        }
    }
}

void jl::Resolver::resolve_function(FuncStmt* stmt, FunctionType function_type)
{
    FunctionType enclosing_function_type = m_current_function_type;
    m_current_function_type = function_type;

    begin_scope();
    for (Token* param : stmt->m_params) {
        declare(*param);
        define(*param);
    }

    resolve(stmt->m_body);
    end_scope();
    m_current_function_type = enclosing_function_type;
}

void jl::Resolver::begin_scope()
{
    m_scopes.push_back(Scope());
}

void jl::Resolver::end_scope()
{
    m_scopes.pop_back();
}

void jl::Resolver::declare(Token& name)
{
    if (m_scopes.empty()) {
        return;
    }

    Scope& scope = m_scopes.back();

    if (scope.contains(name.get_lexeme())) {
        ErrorHandler::error(m_file_name, "resolving", "declaring a variable", name.get_line(), "Another variable of the same name already exists in the same scope", 0);
    }

    scope[name.get_lexeme()] = false;
}

void jl::Resolver::define(Token& name)
{
    if (m_scopes.empty()) {
        return;
    }

    Scope& scope = m_scopes.back();
    scope[name.get_lexeme()] = true;
}

// --------------------------------------------------------------------------------
// -------------------------------Expressions--------------------------------------
// --------------------------------------------------------------------------------

void jl::Resolver::visit_assign_expr(Assign* expr, void* context)
{
    resolve(expr->m_expr);
    resolve_local(expr, expr->m_token);
}

void jl::Resolver::visit_binary_expr(Binary* expr, void* context)
{
    resolve(expr->m_left);
    resolve(expr->m_right);
}

void jl::Resolver::visit_grouping_expr(Grouping* expr, void* context)
{
    resolve(expr->m_expr);
}

void jl::Resolver::visit_unary_expr(Unary* expr, void* context)
{
    resolve(expr->m_expr);
}

void jl::Resolver::visit_literal_expr(Literal* expr, void* context)
{
}

void jl::Resolver::visit_variable_expr(Variable* expr, void* context)
{
    if (!m_scopes.empty() && m_scopes.back().contains(expr->m_name.get_lexeme()) && m_scopes.back().at(expr->m_name.get_lexeme()) == false) {
        ErrorHandler::error(m_file_name, "resolving", "variable expression", expr->m_name.get_line(), "Can't read local variable in its own initializer", 0);
    }

    resolve_local(expr, expr->m_name);
}

void jl::Resolver::visit_logical_expr(Logical* expr, void* context)
{
    resolve(expr->m_left);
    resolve(expr->m_right);
}

void jl::Resolver::visit_call_expr(Call* expr, void* context)
{
    resolve(expr->m_callee);

    for (Expr* arg : expr->m_arguments) {
        resolve(arg);
    }
}

void* jl::Resolver::get_expr_context()
{
    return nullptr;
}

// --------------------------------------------------------------------------------
// -------------------------------Statements---------------------------------------
// --------------------------------------------------------------------------------

void jl::Resolver::visit_print_stmt(PrintStmt* stmt, void* context)
{
    resolve(stmt->m_expr);
}

void jl::Resolver::visit_expr_stmt(ExprStmt* stmt, void* context)
{
    resolve(stmt->m_expr);
}

void jl::Resolver::visit_var_stmt(VarStmt* stmt, void* context)
{
    declare(stmt->m_name);
    if (stmt->m_initializer != nullptr) {
        resolve(stmt->m_initializer);
    }
    define(stmt->m_name);
}

void jl::Resolver::visit_block_stmt(BlockStmt* stmt, void* context)
{
    begin_scope();
    resolve(stmt->m_statements);
    end_scope();
}

void jl::Resolver::visit_empty_stmt(EmptyStmt* stmt, void* context)
{
}

void jl::Resolver::visit_if_stmt(IfStmt* stmt, void* context)
{
    resolve(stmt->m_condition);
    resolve(stmt->m_then_stmt);
    if (stmt->m_else_stmt != nullptr) {
        resolve(stmt->m_else_stmt);
    }
}

void jl::Resolver::visit_while_stmt(WhileStmt* stmt, void* context)
{
    resolve(stmt->m_condition);
    resolve(stmt->m_body);
}

void jl::Resolver::visit_func_stmt(FuncStmt* stmt, void* context)
{
    declare(stmt->m_name);
    define(stmt->m_name);

    resolve_function(stmt, FUNCTION);
}

void jl::Resolver::visit_return_stmt(ReturnStmt* stmt, void* context)
{
    if (m_current_function_type == NONE) {
        ErrorHandler::error(m_file_name, "resolving", "return statement", stmt->m_keyword.get_line(), "Return statement should be inside a function", 0);
    }
    if (stmt->m_expr != nullptr) {
        resolve(stmt->m_expr);
    }
}

void jl::Resolver::visit_class_stmt(ClassStmt* stmt, void* context)
{
    declare(stmt->m_name);
    define(stmt->m_name);
}

void* jl::Resolver::get_stmt_context()
{
    return nullptr;
}
