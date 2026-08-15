#include "Unary.hpp"
#include "backend/ir/IR.hpp"

jl::ir::Unary::Unary(
    value::Variable dest,
    value::Variable operand,
    Operation operation,
    uint32_t line)
    : IR(line)
    , m_dest(dest)
    , m_operand(operand)
    , m_operation(operation)
{
}

std::string jl::ir::Unary::to_str() const
{
    switch (m_operation) {
    case MINUS:
        return m_dest.to_str() + " = - " + m_operand.to_str();
    case BANG:
        return m_dest.to_str() + " = ! " + m_operand.to_str();
    case BIT_NOT:
        return m_dest.to_str() + " = ~ " + m_operand.to_str();
    default:
        return "default-symbol";
    }
}

void jl::ir::Unary::accept(IRVisitor& visitor)
{
    visitor.visit_unary_ir(*this);
}

bool jl::ir::Unary::is_used(value::Variable var)
{
    if (m_operand.id() == var.id()) {
        return true;
    } else {
        return false;
    }
}

std::vector<jl::value::Variable> jl::ir::Unary::uses() const
{
    return { m_operand };
}

void jl::ir::Unary::replace(value::Variable from, value::Variable to)
{

    if (m_dest == from)
        m_dest = to;
    if (m_operand == from)
        m_operand = to;
}

std::optional<jl::value::Variable> jl::ir::Unary::def()
{
    return m_dest;
}