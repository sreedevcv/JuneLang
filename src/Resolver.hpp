#pragma once

#include <map>

#include "Expr.hpp"
#include "Stmt.hpp"
#include "Interpreter.hpp"

namespace jl
{
    
class Resolver: public IExprVisitor, public IStmtVisitor {
public:
    Resolver(Interpreter& interpreter, std::string& file_name);
    ~Resolver() = default;

    void resolve(std::vector<Stmt*>& statements);

    enum FunctionType {
        NONE,
        FUNCTION,
        METHOD,
    };

private:
    using Scope = std::map<std::string, bool>;

    Interpreter& m_interpreter;
    std::vector<Scope> m_scopes;
    std::string& m_file_name;
    FunctionType m_current_function_type = NONE;

    void resolve(Stmt* statement);
    void resolve(Expr* expression);
    void resolve_local(Expr* expr, Token& name);
    void resolve_function(FuncStmt* stmt, FunctionType function_type);  
    void begin_scope();
    void end_scope();
    void declare(Token& name);
    void define(Token& name);

    virtual void visit_assign_expr(Assign* expr, void* context) override;
    virtual void visit_binary_expr(Binary* expr, void* context) override;
    virtual void visit_grouping_expr(Grouping* expr, void* context) override;
    virtual void visit_unary_expr(Unary* expr, void* context) override;
    virtual void visit_literal_expr(Literal* expr, void* context) override;
    virtual void visit_variable_expr(Variable* expr, void* context) override;
    virtual void visit_logical_expr(Logical* expr, void* context) override;
    virtual void visit_call_expr(Call* expr, void* context) override;
    virtual void visit_get_expr(Get* expr, void* context) override;
    virtual void visit_set_expr(Set* expr, void* context) override;
    virtual void visit_this_expr(This* expr, void* context) override;
    virtual void* get_expr_context() override;

    virtual void visit_print_stmt(PrintStmt* stmt, void* context) override;
    virtual void visit_expr_stmt(ExprStmt* stmt, void* context) override;
    virtual void visit_var_stmt(VarStmt* stmt, void* context) override;
    virtual void visit_block_stmt(BlockStmt* stmt, void* context) override;
    virtual void visit_empty_stmt(EmptyStmt* stmt, void* context) override;
    virtual void visit_if_stmt(IfStmt* stmt, void* context) override;
    virtual void visit_while_stmt(WhileStmt* stmt, void* context) override;
    virtual void visit_func_stmt(FuncStmt* stmt, void* context) override;
    virtual void visit_return_stmt(ReturnStmt* stmt, void* context) override;
    virtual void visit_class_stmt(ClassStmt* stmt, void* context) override;
    virtual void* get_stmt_context() override;

};

} // namespace jl
