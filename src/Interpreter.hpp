#pragma once

#include <functional>

#include "Expr.hpp"
#include "Stmt.hpp"
#include "Environment.hpp"

namespace jl {
class Interpreter : public IExprVisitor, public IStmtVisitor {
public:
    Interpreter();
    ~Interpreter();

    void interpret(Expr* expr, Token::Value* value = nullptr);
    void interpret(std::vector<Stmt*>& statements);
    std::string stringify(Token::Value& value);

private:
    virtual void visit_assign_expr(Assign* expr, void* context) override;
    virtual void visit_binary_expr(Binary* expr, void* context) override;
    virtual void visit_grouping_expr(Grouping* expr, void* context) override;
    virtual void visit_unary_expr(Unary* expr, void* context) override;
    virtual void visit_literal_expr(Literal* expr, void* context) override;
    virtual void visit_variable_expr(Variable* expr, void* context) override;
    virtual void visit_logical_expr(Logical* expr, void* context) override;
    virtual void* get_expr_context() override;

    virtual void visit_print_stmt(PrintStmt* stmt, void* context) override;
    virtual void visit_expr_stmt(ExprStmt* stmt, void* context) override;
    virtual void visit_var_stmt(VarStmt* stmt, void* context) override;
    virtual void visit_block_stmt(BlockStmt* stmt, void* context) override;
    virtual void visit_empty_stmt(EmptyStmt* stmt, void* context) override;
    virtual void visit_if_stmt(IfStmt* stmt, void* context) override;
    virtual void* get_stmt_context() override;

    void evaluate(Expr* expr, void* context);
    bool is_truthy(Token::Value* value);

    template<typename Op>
    void do_arith_operation(Token::Value& left, Token::Value& right, void *context, Op op);
    void append_strings(Token::Value& left, Token::Value& right, void* context);
    void execute_block(std::vector<Stmt*>& statements, Environment* new_env);
    bool is_equal(Token::Value& left, Token::Value& right);

    Environment* m_env;
    std::string file_name = "Unknown";
};
} // namespace jl
