#pragma once

#include <functional>
#include <map>

#include "Expr.hpp"
#include "Stmt.hpp"
#include "Environment.hpp"

namespace jl {
class Interpreter : public IExprVisitor, public IStmtVisitor {
public:
    Interpreter(std::string& file_name);
    ~Interpreter();

    void interpret(Expr* expr, Value* value = nullptr);
    void interpret(std::vector<Stmt*>& statements);
    void resolve(Expr* expr, int depth);
    void execute_block(std::vector<Stmt*>& statements, Environment* new_env);
    std::string stringify(Value& value);

    Environment* m_global_env;

private:
    virtual void visit_assign_expr(Assign* expr, void* context) override;
    virtual void visit_binary_expr(Binary* expr, void* context) override;
    virtual void visit_grouping_expr(Grouping* expr, void* context) override;
    virtual void visit_unary_expr(Unary* expr, void* context) override;
    virtual void visit_literal_expr(Literal* expr, void* context) override;
    virtual void visit_variable_expr(Variable* expr, void* context) override;
    virtual void visit_logical_expr(Logical* expr, void* context) override;
    virtual void visit_call_expr(Call* expr, void* context) override;
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

    void evaluate(Expr* expr, void* context);
    bool is_truthy(Value* value);

    template<typename Op>
    void do_arith_operation(Value& left, Value& right, void *context, Op op);
    void append_strings(Value& left, Value& right, void* context);
    bool is_equal(Value& left, Value& right);
    Value& look_up_variable(Token& name, Expr* expr);

    Environment* m_env;
    std::string m_file_name;
    std::map<Expr*, int> m_locals;

    friend class ToIntNativeFunction;
};
} // namespace jl
