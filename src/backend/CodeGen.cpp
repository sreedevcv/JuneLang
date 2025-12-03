#include "CodeGen.hpp"

#include "Utils.hpp"
#include "Value.hpp"
#include "backend/ir/TypeCast.hpp"
#include "backend/ir/Unary.hpp"
#include "backend/types/Type.hpp"
#include <any>
#include <memory>
#include <vector>

std::vector<std::unique_ptr<jl::type::Type>> vec;
auto void_func = std::make_unique<jl::type::Func>(
    std::move(std::make_unique<jl::type::Builtin>(jl::type::Builtin::VOID)),
    std::move(vec));

jl::CodeGen::CodeGen()
    : m_block(nullptr, 0)
    , m_func(std::string("__root__"), std::move(void_func))
{
}

jl::FuncBlock jl::CodeGen::generate(Expr* expr)
{
    emit(expr);
    return std::move(m_func);
}

std::shared_ptr<jl::value::Variable> jl::CodeGen::emit(Expr* expr)
{
    auto var = std::any_cast<std::shared_ptr<value::Variable>>(expr->accept(*this));

    // Insert typecast ir here
    if (expr->m_cast_to) {
        auto dest = m_block.create_varaible();
        m_func.add_ir<ir::TypeCast>(
            expr->m_type->clone(),
            expr->m_cast_to.value()->clone(),
            dest,
            var,
            m_func.get_last_line());

        var = dest;
    }

    return var;
}

void jl::CodeGen::emit(Stmt* stmt)
{
    stmt->accept(*this);
}

void jl::CodeGen::emit(std::vector<std::unique_ptr<Stmt>>& stmts)
{
    for (auto& stmt : stmts) {
        emit(stmt.get());
    }
}

std::any jl::CodeGen::visit_assign_expr(Assign* expr)
{
    unimplemented("Codegen");
    return {};
}
std::any jl::CodeGen::visit_binary_expr(Binary* expr)
{
    unimplemented("Codegen");
    return {};
}
std::any jl::CodeGen::visit_grouping_expr(Grouping* expr)
{
    unimplemented("Codegen");
    return {};
}
std::any jl::CodeGen::visit_unary_expr(Unary* expr)
{
    ir::Unary::Operation operation;

    switch (expr->m_oper.get_tokentype()) {
    case Token::MINUS:
        operation = ir::Unary::MINUS;
        break;
    case Token::BANG:
        operation = ir::Unary::BANG;
        break;
    case Token::BIT_NOT:
        operation = ir::Unary::BIT_NOT;
        break;
    default:
        unimplemented();
    }

    const auto source = emit(expr->m_expr.get());
    const auto dest = m_block.create_varaible();
    m_func.add_ir<ir::Unary>(dest, source, operation, expr->m_oper.get_line());
    return dest;
}

std::any jl::CodeGen::visit_literal_expr(Literal* expr)
{
    auto& value = expr->m_value;

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

    unimplemented("Codegen");
    return {};
}

std::any jl::CodeGen::visit_variable_expr(Variable* expr)
{
    unimplemented("Codegen");
    return {};
}
std::any jl::CodeGen::visit_logical_expr(Logical* expr)
{
    unimplemented("Codegen");
    return {};
}
std::any jl::CodeGen::visit_call_expr(Call* expr)
{
    unimplemented("Codegen");
    return {};
}
std::any jl::CodeGen::visit_get_expr(Get* expr)
{
    unimplemented("Codegen");
    return {};
}
std::any jl::CodeGen::visit_set_expr(Set* expr)
{
    unimplemented("Codegen");
    return {};
}
std::any jl::CodeGen::visit_this_expr(This* expr)
{
    unimplemented("Codegen");
    return {};
}
std::any jl::CodeGen::visit_super_expr(Super* expr)
{
    unimplemented("Codegen");
    return {};
}
std::any jl::CodeGen::visit_jlist_expr(JList* expr)
{
    unimplemented("Codegen");
    return {};
}
std::any jl::CodeGen::visit_index_get_expr(IndexGet* expr)
{
    unimplemented("Codegen");
    return {};
}
std::any jl::CodeGen::visit_index_set_expr(IndexSet* expr)
{
    unimplemented("Codegen");
    return {};
}
std::any jl::CodeGen::visit_type_cast_expr(TypeCast* expr)
{
    unimplemented("Codegen");
    return {};
}

std::any jl::CodeGen::visit_print_stmt(PrintStmt* stmt)
{
    unimplemented("Codegen");
    return {};
}
std::any jl::CodeGen::visit_expr_stmt(ExprStmt* stmt)
{
    unimplemented("Codegen");
    return {};
}
std::any jl::CodeGen::visit_var_stmt(VarStmt* stmt)
{
    unimplemented("Codegen");
    return {};
}
std::any jl::CodeGen::visit_block_stmt(BlockStmt* stmt)
{
    unimplemented("Codegen");
    return {};
}
std::any jl::CodeGen::visit_empty_stmt(EmptyStmt* stmt)
{
    unimplemented("Codegen");
    return {};
}
std::any jl::CodeGen::visit_if_stmt(IfStmt* stmt)
{
    unimplemented("Codegen");
    return {};
}
std::any jl::CodeGen::visit_while_stmt(WhileStmt* stmt)
{
    unimplemented("Codegen");
    return {};
}
std::any jl::CodeGen::visit_func_stmt(FuncStmt* stmt)
{
    unimplemented("Codegen");
    return {};
}
std::any jl::CodeGen::visit_return_stmt(ReturnStmt* stmt)
{
    unimplemented("Codegen");
    return {};
}
std::any jl::CodeGen::visit_class_stmt(ClassStmt* stmt)
{
    unimplemented("Codegen");
    return {};
}
std::any jl::CodeGen::visit_for_each_stmt(ForEachStmt* stmt)
{
    unimplemented("Codegen");
    return {};
}
std::any jl::CodeGen::visit_break_stmt(BreakStmt* stmt)
{
    unimplemented("Codegen");
    return {};
}
std::any jl::CodeGen::visit_extern_stmt(ExternStmt* stmt)
{
    unimplemented("Codegen");
    return {};
}
