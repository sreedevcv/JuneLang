#pragma once

#include <map>
#include <vector>

#include "Callable.hpp"
#include "Environment.hpp"
#include "Expr.hpp"
#include "GarbageCollector.hpp"
#include "Stmt.hpp"

namespace jl {

class Interpreter : public IExprVisitor, public IStmtVisitor {
public:
    Interpreter(std::string& file_name);
    virtual ~Interpreter();

    void interpret(Expr* expr, Value* value);
    void interpret(std::vector<Stmt*>& statements);
    void resolve(Expr* expr, int depth);
    void execute_block(std::vector<Stmt*>& statements, Environment* new_env);
    std::string stringify(Value* value);

    Environment* m_global_env { nullptr };
    std::string m_file_name;

private:
    virtual std::any visit_assign_expr(Assign* expr) override;
    virtual std::any visit_binary_expr(Binary* expr) override;
    virtual std::any visit_grouping_expr(Grouping* expr) override;
    virtual std::any visit_unary_expr(Unary* expr) override;
    virtual std::any visit_literal_expr(Literal* expr) override;
    virtual std::any visit_variable_expr(Variable* expr) override;
    virtual std::any visit_logical_expr(Logical* expr) override;
    virtual std::any visit_call_expr(Call* expr) override;
    virtual std::any visit_get_expr(Get* expr) override;
    virtual std::any visit_set_expr(Set* expr) override;
    virtual std::any visit_this_expr(This* expr) override;
    virtual std::any visit_super_expr(Super* expr) override;
    virtual std::any visit_jlist_expr(JList* expr) override;
    virtual std::any visit_index_get_expr(IndexGet* expr) override;
    virtual std::any visit_index_set_expr(IndexSet* expr) override;

    virtual std::any visit_print_stmt(PrintStmt* stmt) override;
    virtual std::any visit_expr_stmt(ExprStmt* stmt) override;
    virtual std::any visit_var_stmt(VarStmt* stmt) override;
    virtual std::any visit_block_stmt(BlockStmt* stmt) override;
    virtual std::any visit_empty_stmt(EmptyStmt* stmt) override;
    virtual std::any visit_if_stmt(IfStmt* stmt) override;
    virtual std::any visit_while_stmt(WhileStmt* stmt) override;
    virtual std::any visit_func_stmt(FuncStmt* stmt) override;
    virtual std::any visit_return_stmt(ReturnStmt* stmt) override;
    virtual std::any visit_class_stmt(ClassStmt* stmt) override;
    virtual std::any visit_for_each_stmt(ForEachStmt* stmt) override;
    virtual std::any visit_break_stmt(BreakStmt* stmt) override;

    Value* evaluate(Expr* expr);
    bool is_truthy(Value* value);

    template <typename Op>
    Value* do_arith_operation(Value* left, Value* right, Op op, int line, bool is_logical=false);
    Value* append_strings(Value* left, Value* right);
    bool is_equal(Value* left, Value* right);
    Value* look_up_variable(Token& name, Expr* expr);

    Environment m_dummy_env;
    Environment* m_env { nullptr };
    std::vector<Environment*> m_env_stack;
    std::vector<Ref*> m_allocated_refs;
    GarbageCollector m_gc;
    std::map<Expr*, int> m_locals;
    struct BreakThrow { };

    friend class ClassCallable;
    friend class FunctionCallable;
    friend class Instance;
    friend class ToStrNativeFunction;
    friend class ToIntNativeFunction;

    friend Value* jlist_clear(Interpreter* interpreter, Value* jlist);
    friend Value* jlist_get_len(Interpreter* interpreter, std::string& file_name, Value* jlist);
    friend Value* jlist_push_back(Interpreter* interpreter, Value* jlist, Value* appending_value);
    friend Value* jlist_pop_back(Interpreter* interpreter, Value* jlist);
};
} // namespace jl
