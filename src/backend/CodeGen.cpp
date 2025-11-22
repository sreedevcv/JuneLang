#include "CodeGen.hpp"

#include "Utils.hpp"
#include "Value.hpp"

jl::CodeGen::CodeGen()
    : m_block(nullptr, 0, "__root__")
{
}

jl::FuncBlock jl::CodeGen::generate(Expr* expr)
{
    expr->accept(*this);

    return std::move(m_block);
}

std::any jl::CodeGen::visit_assign_expr(Assign* expr) { }
std::any jl::CodeGen::visit_binary_expr(Binary* expr) { }
std::any jl::CodeGen::visit_grouping_expr(Grouping* expr) { }
std::any jl::CodeGen::visit_unary_expr(Unary* expr) { }

std::any jl::CodeGen::visit_literal_expr(Literal* expr)
{
    auto value = expr->m_value;

    switch (get_type(value)) {
    case Type::INT:
        return add_literal_ir<int>(value);
    case Type::FLOAT:
        return add_literal_ir<double>(value);
    case Type::STR:
    case Type::BOOL:
    case Type::CHAR:
    case Type::JNULL:
    default:
        unimplemented();
        break;
    }
}

std::any jl::CodeGen::visit_variable_expr(Variable* expr) { }
std::any jl::CodeGen::visit_logical_expr(Logical* expr) { }
std::any jl::CodeGen::visit_call_expr(Call* expr) { }
std::any jl::CodeGen::visit_get_expr(Get* expr) { }
std::any jl::CodeGen::visit_set_expr(Set* expr) { }
std::any jl::CodeGen::visit_this_expr(This* expr) { }
std::any jl::CodeGen::visit_super_expr(Super* expr) { }
std::any jl::CodeGen::visit_jlist_expr(JList* expr) { }
std::any jl::CodeGen::visit_index_get_expr(IndexGet* expr) { }
std::any jl::CodeGen::visit_index_set_expr(IndexSet* expr) { }
std::any jl::CodeGen::visit_type_cast_expr(TypeCast* expr) { }

std::any jl::CodeGen::visit_print_stmt(PrintStmt* stmt) { }
std::any jl::CodeGen::visit_expr_stmt(ExprStmt* stmt) { }
std::any jl::CodeGen::visit_var_stmt(VarStmt* stmt) { }
std::any jl::CodeGen::visit_block_stmt(BlockStmt* stmt) { }
std::any jl::CodeGen::visit_empty_stmt(EmptyStmt* stmt) { }
std::any jl::CodeGen::visit_if_stmt(IfStmt* stmt) { }
std::any jl::CodeGen::visit_while_stmt(WhileStmt* stmt) { }
std::any jl::CodeGen::visit_func_stmt(FuncStmt* stmt) { }
std::any jl::CodeGen::visit_return_stmt(ReturnStmt* stmt) { }
std::any jl::CodeGen::visit_class_stmt(ClassStmt* stmt) { }
std::any jl::CodeGen::visit_for_each_stmt(ForEachStmt* stmt) { }
std::any jl::CodeGen::visit_break_stmt(BreakStmt* stmt) { }
std::any jl::CodeGen::visit_extern_stmt(ExternStmt* stmt) { }
