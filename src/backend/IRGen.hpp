#pragma once

#include "Expr.hpp"
#include "Stmt.hpp"
#include "Value.hpp"
#include "backend/Block.hpp"
#include "backend/FuncBlock.hpp"
#include "backend/LiteralValue.hpp"
#include "backend/ir/InitLiteral.hpp"
#include "backend/value/Variable.hpp"

#include <memory>
#include <unordered_map>

namespace jl {
class IRGen : IExprVisitor, IStmtVisitor {
public:
    IRGen();

    FuncBlock generate(Expr* expr);
    FuncBlock generate(std::vector<std::unique_ptr<jl::Stmt>>& stmts);

private:
    Block* m_block;

    FuncBlock m_func;

    value::Variable emit(Expr* expr);

    std::stack<Block> m_env;

    std::unordered_map<value::Variable, std::string> m_func_vars;

    void emit(Stmt* stmt);

    void emit(std::vector<std::unique_ptr<Stmt>>& stmts);

    template <typename T>
    jl::value::Variable add_literal_ir(Value value, const type::Type* size)
    {
        const auto val = std::get<T>(value);
        const auto var = m_block->create_varaible(m_func.get_current_func_name(), size);
        auto literal = std::make_unique<LiteralValue>(val);
        m_func.add_ir<ir::InitLiteral>(std::move(literal), var, m_func.get_last_line());
        return var;
    }

    void push_block();

    void pop_block();

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
