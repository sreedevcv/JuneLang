#include "CodeGenerator.hpp"
#include "Stmt.hpp"
#include <string>

jl::CodeGenerator::CodeGenerator(std::string& file_name)
    : m_file_name(file_name)
{
}

jl::CodeGenerator::~CodeGenerator()
{
}

void jl::CodeGenerator::generate(std::vector<Stmt*> stmts)
{
    for (auto stmt : stmts) {
        visit(stmt);
    }
}

void jl::CodeGenerator::visit(Stmt* stmt)
{
    stmt->accept(*this);
}

/* ======================================================================================================================== */
/* -----------------------------------------------------EXPR--------------------------------------------------------------- */
/* ======================================================================================================================== */

std::any jl::CodeGenerator::visit_assign_expr(Assign* expr) { }

std::any jl::CodeGenerator::visit_binary_expr(Binary* expr)
{
}

std::any jl::CodeGenerator::visit_grouping_expr(Grouping* expr)
{
}

std::any jl::CodeGenerator::visit_unary_expr(Unary* expr)
{
}

std::any jl::CodeGenerator::visit_literal_expr(Literal* expr)
{
    m_byte_code.push_back(std::string { "PUSH " } + to_string(expr->m_value));
}

std::any jl::CodeGenerator::visit_variable_expr(Variable* expr) { }
std::any jl::CodeGenerator::visit_logical_expr(Logical* expr) { }
std::any jl::CodeGenerator::visit_call_expr(Call* expr) { }
std::any jl::CodeGenerator::visit_get_expr(Get* expr) { }
std::any jl::CodeGenerator::visit_set_expr(Set* expr) { }
std::any jl::CodeGenerator::visit_this_expr(This* expr) { }
std::any jl::CodeGenerator::visit_super_expr(Super* expr) { }
std::any jl::CodeGenerator::visit_jlist_expr(JList* expr) { }
std::any jl::CodeGenerator::visit_index_get_expr(IndexGet* expr) { }
std::any jl::CodeGenerator::visit_index_set_expr(IndexSet* expr) { }

std::any jl::CodeGenerator::visit_print_stmt(PrintStmt* stmt) { }
std::any jl::CodeGenerator::visit_expr_stmt(ExprStmt* stmt) { }
std::any jl::CodeGenerator::visit_var_stmt(VarStmt* stmt) { }
std::any jl::CodeGenerator::visit_block_stmt(BlockStmt* stmt) { }
std::any jl::CodeGenerator::visit_empty_stmt(EmptyStmt* stmt) { }
std::any jl::CodeGenerator::visit_if_stmt(IfStmt* stmt) { }
std::any jl::CodeGenerator::visit_while_stmt(WhileStmt* stmt) { }
std::any jl::CodeGenerator::visit_func_stmt(FuncStmt* stmt) { }
std::any jl::CodeGenerator::visit_return_stmt(ReturnStmt* stmt) { }
std::any jl::CodeGenerator::visit_class_stmt(ClassStmt* stmt) { }
std::any jl::CodeGenerator::visit_for_each_stmt(ForEachStmt* stmt) { }
std::any jl::CodeGenerator::visit_break_stmt(BreakStmt* stmt) { }