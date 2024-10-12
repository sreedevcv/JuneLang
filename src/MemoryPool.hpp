#pragma once

#include <type_traits>
#include <utility>

#include "Environment.hpp"
#include "Expr.hpp"
#include "Ref.hpp"
#include "Stmt.hpp"
#include "Value.hpp"

namespace jl {

template <typename T>
concept CanBeRef = std::is_base_of<Ref, T>::value;

class MemoryPool : public IExprVisitor, public IStmtVisitor {

public:
    MemoryPool();
    ~MemoryPool() = default;

    template <CanBeRef T, typename... Args>
    T* allocate(Args&&... args)
    {
        T* obj = new T(std::forward(args...));
        obj->next = head->next;
        head->next = obj;
        return obj;
    }

    void mark(Expr* epxr);
    void mark(Stmt* stmt);
    void mark(JlValue* value);
    void mark(Callable* callable);
    void mark(Environment* env);

private:
    Ref dummy_ref;
    Ref* head { nullptr };

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
};

}