#include "Unary.hpp"
#include "backend/ir/IR.hpp"

jl::ir::Unary::Unary(
    std::shared_ptr<value::Variable> dest,
    std::shared_ptr<value::Variable> operand,
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
        return m_dest->to_str() + " = - " + m_operand->to_str();
    case BANG:
        return m_dest->to_str() + " = ! " + m_operand->to_str();
    case BIT_NOT:
        return m_dest->to_str() + " = ~ " + m_operand->to_str();
    }
}

void jl::ir::Unary::accept(IRVisitor& visitor)
{
    visitor.visit_unary_ir(*this);
}
