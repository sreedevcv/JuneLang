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
    statement->accept(*this);
}

void jl::Resolver::resolve(Expr* expression)
{
    expression->accept(*this);
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

std::any jl::Resolver::visit_assign_expr(Assign* expr)
{
    resolve(expr->m_expr);
    resolve_local(expr, expr->m_token);
    return nullptr;
}

std::any jl::Resolver::visit_binary_expr(Binary* expr)
{
    resolve(expr->m_left);
    resolve(expr->m_right);
    return nullptr;
}

std::any jl::Resolver::visit_grouping_expr(Grouping* expr)
{
    resolve(expr->m_expr);
    return nullptr;
}

std::any jl::Resolver::visit_unary_expr(Unary* expr)
{
    resolve(expr->m_expr);
    return nullptr;
}

std::any jl::Resolver::visit_literal_expr(Literal* expr)
{
    return nullptr;
}

std::any jl::Resolver::visit_variable_expr(Variable* expr)
{
    if (!m_scopes.empty() && m_scopes.back().contains(expr->m_name.get_lexeme()) && m_scopes.back().at(expr->m_name.get_lexeme()) == false) {
        ErrorHandler::error(m_file_name, "resolving", "variable expression", expr->m_name.get_line(), "Can't read local variable in its own initializer", 0);
    }

    resolve_local(expr, expr->m_name);
    return nullptr;
}

std::any jl::Resolver::visit_logical_expr(Logical* expr)
{
    resolve(expr->m_left);
    resolve(expr->m_right);
    return nullptr;
}

std::any jl::Resolver::visit_call_expr(Call* expr)
{
    resolve(expr->m_callee);

    for (Expr* arg : expr->m_arguments) {
        resolve(arg);
    }
    return nullptr;
}

std::any jl::Resolver::visit_get_expr(Get* expr)
{
    resolve(expr->m_object);
    return nullptr;
}

std::any jl::Resolver::visit_set_expr(Set* expr)
{
    resolve(expr->m_object);
    resolve(expr->m_value);
    return nullptr;
}

std::any jl::Resolver::visit_this_expr(This* expr)
{
    if (m_current_class_type == ClassType::NONE) {
        ErrorHandler::error(m_file_name, "resolving", "self keyword", expr->m_keyword.get_line(), "Cannot use 'self' outside a class", 0);
    }
    resolve_local(expr, expr->m_keyword);
    return nullptr;
}

std::any jl::Resolver::visit_super_expr(Super* expr)
{
    if (m_current_class_type == ClassType::NONE) {
        ErrorHandler::error(m_file_name, "resolving", "super", expr->m_keyword.get_line(), "Super keyword should be used within a class", 0);
    } else if (m_current_class_type != ClassType::SUBCLASS) {
        ErrorHandler::error(m_file_name, "resolving", "super", expr->m_keyword.get_line(), "Super keyword should be used within a sub-class", 0);
    }
    resolve_local(expr, expr->m_keyword);
    return nullptr;
}

std::any jl::Resolver::visit_jlist_expr(JList* expr)
{
    for (Expr* item : expr->m_items) {
        resolve(item);
    }
    return nullptr;
}

std::any jl::Resolver::visit_index_get_expr(IndexGet* expr)
{
    resolve(expr->m_jlist);
    resolve(expr->m_index_expr);
    return nullptr;
}

std::any jl::Resolver::visit_index_set_expr(IndexSet* expr)
{
    resolve(expr->m_jlist);
    resolve(expr->m_index_expr);
    resolve(expr->m_value_expr);
    return nullptr;
}

// --------------------------------------------------------------------------------
// -------------------------------Statements---------------------------------------
// --------------------------------------------------------------------------------

std::any jl::Resolver::visit_print_stmt(PrintStmt* stmt)
{
    resolve(stmt->m_expr);
    return nullptr;
}

std::any jl::Resolver::visit_expr_stmt(ExprStmt* stmt)
{
    resolve(stmt->m_expr);
    return nullptr;
}

std::any jl::Resolver::visit_var_stmt(VarStmt* stmt)
{
    declare(stmt->m_name);
    if (stmt->m_initializer != nullptr) {
        resolve(stmt->m_initializer);
    }
    define(stmt->m_name);
    return nullptr;
}

std::any jl::Resolver::visit_block_stmt(BlockStmt* stmt)
{
    begin_scope();
    resolve(stmt->m_statements);
    end_scope();
    return nullptr;
}

std::any jl::Resolver::visit_empty_stmt(EmptyStmt* stmt)
{
    return nullptr;
}

std::any jl::Resolver::visit_if_stmt(IfStmt* stmt)
{
    resolve(stmt->m_condition);
    resolve(stmt->m_then_stmt);
    if (stmt->m_else_stmt != nullptr) {
        resolve(stmt->m_else_stmt);
    }
    return nullptr;
}

std::any jl::Resolver::visit_while_stmt(WhileStmt* stmt)
{
    LoopType enclosing_loop_type = m_current_loop_type;

    resolve(stmt->m_condition);

    m_current_loop_type = LoopType::LOOP;
        resolve(stmt->m_body);
    m_current_loop_type = enclosing_loop_type;

    return nullptr;
}

std::any jl::Resolver::visit_func_stmt(FuncStmt* stmt)
{
    declare(stmt->m_name);
    define(stmt->m_name);

    resolve_function(stmt, FunctionType::FUNCTION);
    return nullptr;
}

std::any jl::Resolver::visit_return_stmt(ReturnStmt* stmt)
{
    if (m_current_function_type == FunctionType::NONE) {
        ErrorHandler::error(m_file_name, "resolving", "return statement", stmt->m_keyword.get_line(), "Return statement should be inside a function", 0);
    }
    if (stmt->m_expr != nullptr) {
        if (m_current_function_type == FunctionType::INITIALIZER) {
            ErrorHandler::error(m_file_name, "resolving", "return", stmt->m_keyword.get_line(), "Can't return a value from an initializer", 0);
        }
        resolve(stmt->m_expr);
    }
    return nullptr;
}

std::any jl::Resolver::visit_class_stmt(ClassStmt* stmt)
{
    ClassType enclosing_class = m_current_class_type;
    m_current_class_type = ClassType::CLASS;

    declare(stmt->m_name);
    define(stmt->m_name);

    if (stmt->m_super_class != nullptr && stmt->m_name.get_lexeme() == stmt->m_super_class->m_name.get_lexeme()) {
        ErrorHandler::error(m_file_name, "resolving", "class definition", stmt->m_name.get_line(), "A class cannot inherit from itself", 0);
    }

    if (stmt->m_super_class != nullptr) {
        m_current_class_type = ClassType::SUBCLASS;
        resolve(stmt->m_super_class);
        begin_scope();
        m_scopes.back()[Token::global_super_lexeme] = true;
    }

    begin_scope();
    m_scopes.back()[Token::global_this_lexeme] = true;

    for (FuncStmt* method : stmt->m_methods) {
        FunctionType declaration = FunctionType::METHOD;
        if (method->m_name.get_lexeme() == "init") {
            declaration = FunctionType::INITIALIZER;
        }
        resolve_function(method, declaration);
    }

    end_scope();

    if (stmt->m_super_class != nullptr) {
        end_scope();
    }
    m_current_class_type = enclosing_class;
    return nullptr;
}

std::any jl::Resolver::visit_for_each_stmt(ForEachStmt* stmt)
{   
    LoopType enclosing_loop_type = m_current_loop_type;

    begin_scope();
    resolve(stmt->m_var_declaration);
    resolve(stmt->m_list_expr);

    m_current_loop_type = LoopType::LOOP;
        resolve(stmt->m_body);
    m_current_loop_type = enclosing_loop_type;

    end_scope();
    return nullptr;
}

std::any jl::Resolver::visit_break_stmt(BreakStmt* stmt)
{
    if (m_current_loop_type == LoopType::NONE) {
        ErrorHandler::error(m_file_name, "resolving", "break statement", stmt->m_break_token.get_line(), "Break statement should be inside a loop", 0);
    }
    return nullptr;
}
