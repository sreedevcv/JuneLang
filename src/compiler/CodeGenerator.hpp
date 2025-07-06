#pragma once

#include "Chunk.hpp"
#include "DataSection.hpp"
#include "Expr.hpp"
#include "Operand.hpp"
#include "Stmt.hpp"

#include <any>
#include <map>
#include <stack>
#include <string>
#include <vector>

namespace jl {
class CodeGenerator : public IExprVisitor, public IStmtVisitor {
public:
    CodeGenerator(std::string& file_name);
    virtual ~CodeGenerator();

    const std::pair<std::map<std::string, Chunk>&, DataSection&> generate(std::vector<Stmt*> stmts);

    jl::Operand compile(Stmt* stmt);
    jl::Operand compile(Expr* stmt);

    void disassemble();
    const Chunk& get_root_chunk() const;

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

    std::string m_file_name;
    Chunk* m_chunk;
    std::map<std::string, Chunk> m_chunk_list;
    std::stack<Chunk*> m_func_stack;
    DataSection data_section;

    bool check_if_func_exists(const std::string& name) const;
    void push_chunk(Chunk&& chunk, const std::string& name);
    void pop_chunk();
};

} // namespace jl
