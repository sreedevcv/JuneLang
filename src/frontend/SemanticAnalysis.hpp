#pragma once

#include "Expr.hpp"
#include "Stmt.hpp"
#include "Value.hpp"
#include "types/Type.hpp"

#include <memory>
#include <optional>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

namespace jl {
class SemanticAnalyzer : IExprVisitor, IStmtVisitor {
public:
    SemanticAnalyzer(std::string& file_name);

    bool type_check(Expr* expr);
    bool type_check(Stmt* stmt);
    bool type_check(const std::vector<std::unique_ptr<Stmt>>& stmts);

private:
    std::string m_file_name = "KLKLKL";
    std::vector<std::unordered_map<std::string, std::optional<std::unique_ptr<type::Type>>>> m_symbol_table;
    std::stack<const type::Type*> m_func_types;


    bool is_defined(const std::string& name);
    std::optional<std::unique_ptr<type::Type>>& get_variable_type(const std::string& name);
    bool define_variable(const std::string& name, std::optional<std::unique_ptr<type::Type>> type);

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
