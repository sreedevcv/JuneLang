#pragma once

#include <map>
#include <stack>
#include <vector>

#include "Arena.hpp"
#include "Callable.hpp"
#include "Environment.hpp"
#include "Expr.hpp"
#include "GarbageCollector.hpp"
#include "MemoryPool.hpp"
#include "Stmt.hpp"

namespace jl {

class Interpreter : public IExprVisitor, public IStmtVisitor {
public:
    Interpreter(Arena& arena, std::string& file_name, int64_t internal_arena_size = 1000 * 2000);
    virtual ~Interpreter();

    void interpret(Expr* expr, JlValue* value = nullptr);
    void interpret(std::vector<Stmt*>& statements);
    void resolve(Expr* expr, int depth);
    void execute_block(std::vector<Stmt*>& statements, Environment* new_env);
    std::string stringify(JlValue* value);

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

    JlValue* evaluate(Expr* expr);
    bool is_truthy(JlValue* value);

    template <typename Op>
    JlValue* do_arith_operation(JlValue* left, JlValue* right, Op op, int line, bool is_logical=false);
    JlValue* append_strings(JlValue* left, JlValue* right);
    bool is_equal(JlValue* left, JlValue* right);
    JlValue* look_up_variable(Token& name, Expr* expr);

    Environment m_dummy_env;
    Arena& m_arena;
    Arena m_internal_arena;
    Environment* m_env { nullptr };
    std::vector<Environment*> m_env_stack;
    GarbageCollector m_gc;
    std::map<Expr*, int> m_locals;
    struct BreakThrow { };

    friend class ClassCallable;
    friend class FunctionCallable;
    friend class Instance;
    friend class ToStrNativeFunction;
    friend class ToIntNativeFunction;

    friend JlValue* jlist_clear(Interpreter* interpreter, JlValue* jlist);
    friend JlValue* jlist_get_len(Interpreter* interpreter, std::string& file_name, JlValue* jlist);
    friend JlValue* jlist_push_back(Interpreter* interpreter, JlValue* jlist, JlValue* appending_value);
    friend JlValue* jlist_pop_back(Interpreter* interpreter, JlValue* jlist);
};
} // namespace jl
