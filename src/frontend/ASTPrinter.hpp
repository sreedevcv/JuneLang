#pragma once

#include "Expr.hpp"
#include "Stmt.hpp"
#include <ostream>
#include <sstream>

namespace jl {
class ASTPrinter : IExprVisitor, IStmtVisitor {
public:
    ASTPrinter();

    std::stringstream print(Expr* epxr);
    std::stringstream print(const std::vector<std::unique_ptr<Stmt>>& stms);

    private:
    int increment = 3;
    int depth = -increment;
    std::stringstream stream;

    std::ostream& spacer();
    void traverse(Expr* epxr);
    void traverse(Stmt* stmt);

    std::any visit_assign_expr(Assign* expr) override;
    std::any visit_binary_expr(Binary* expr) override;
    std::any visit_grouping_expr(Grouping* expr) override;
    std::any visit_unary_expr(Unary* expr) override;
    std::any visit_literal_expr(Literal* expr) override;
    std::any visit_variable_expr(Variable* expr) override;
    std::any visit_logical_expr(Logical* expr) override;
    std::any visit_call_expr(Call* expr) override;
    std::any visit_get_expr(Get* expr) override;
    std::any visit_set_expr(Set* expr) override;
    std::any visit_this_expr(This* expr) override;
    std::any visit_super_expr(Super* expr) override;
    std::any visit_jlist_expr(JList* expr) override;
    std::any visit_index_get_expr(IndexGet* expr) override;
    std::any visit_index_set_expr(IndexSet* expr) override;
    std::any visit_type_cast_expr(TypeCast* expr) override;

    std::any visit_print_stmt(PrintStmt* stmt) override;
    std::any visit_expr_stmt(ExprStmt* stmt) override;
    std::any visit_var_stmt(VarStmt* stmt) override;
    std::any visit_block_stmt(BlockStmt* stmt) override;
    std::any visit_empty_stmt(EmptyStmt* stmt) override;
    std::any visit_if_stmt(IfStmt* stmt) override;
    std::any visit_while_stmt(WhileStmt* stmt) override;
    std::any visit_func_stmt(FuncStmt* stmt) override;
    std::any visit_return_stmt(ReturnStmt* stmt) override;
    std::any visit_class_stmt(ClassStmt* stmt) override;
    std::any visit_for_each_stmt(ForEachStmt* stmt) override;
    std::any visit_break_stmt(BreakStmt* stmt) override;
    std::any visit_extern_stmt(ExternStmt* stmt) override;
};
}
