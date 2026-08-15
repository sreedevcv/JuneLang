#include "Binary.hpp"

jl::ir::Binary::Binary(
    value::Variable dest,
    value::Variable operand_a,
    value::Variable operand_b,
    Operation operation,
    bool is_float,
    uint32_t line)
    : IR(line)
    , m_dest(dest)
    , m_operand_a(operand_a)
    , m_operand_b(operand_b)
    , m_operation(operation)
    , m_is_float(is_float)
{
}

std::string jl::ir::Binary::to_str() const
{
    switch (m_operation) {
    case PLUS:
        return m_dest.to_str() + " = " + m_operand_a.to_str() + " + " + m_operand_b.to_str();
    case MINUS:
        return m_dest.to_str() + " = " + m_operand_a.to_str() + " - " + m_operand_b.to_str();
    case STAR:
        return m_dest.to_str() + " = " + m_operand_a.to_str() + " * " + m_operand_b.to_str();
    case SLASH:
        return m_dest.to_str() + " = " + m_operand_a.to_str() + " / " + m_operand_b.to_str();
    case GREATER:
        return m_dest.to_str() + " = " + m_operand_a.to_str() + " > " + m_operand_b.to_str();
    case LESS:
        return m_dest.to_str() + " = " + m_operand_a.to_str() + " < " + m_operand_b.to_str();
    case GREATER_EQUAL:
        return m_dest.to_str() + " = " + m_operand_a.to_str() + " >= " + m_operand_b.to_str();
    case LESS_EQUAL:
        return m_dest.to_str() + " = " + m_operand_a.to_str() + " <= " + m_operand_b.to_str();
    case EQUAL_EQUAL:
        return m_dest.to_str() + " = " + m_operand_a.to_str() + " == " + m_operand_b.to_str();
    case BANG_EQUAL:
        return m_dest.to_str() + " = " + m_operand_a.to_str() + " != " + m_operand_b.to_str();
    case PERCENT:
        return m_dest.to_str() + " = " + m_operand_a.to_str() + " % " + m_operand_b.to_str();
    case BIT_AND:
        return m_dest.to_str() + " = " + m_operand_a.to_str() + " & " + m_operand_b.to_str();
    case BIT_OR:
        return m_dest.to_str() + " = " + m_operand_a.to_str() + " | " + m_operand_b.to_str();
    case BIT_XOR:
        return m_dest.to_str() + " = " + m_operand_a.to_str() + " ^ " + m_operand_b.to_str();
    case LOG_AND:
        return m_dest.to_str() + " = " + m_operand_a.to_str() + " && " + m_operand_b.to_str();
    case LOG_OR:
        return m_dest.to_str() + " = " + m_operand_a.to_str() + " || " + m_operand_b.to_str();
    default:
        return "default-symbol";
    }
}

void jl::ir::Binary::accept(IRVisitor& visitor)
{
    visitor.visit_binary_ir(*this);
}

bool jl::ir::Binary::is_used(value::Variable var)
{
    if (m_operand_a.id() == var.id() || m_operand_b.id() == var.id()) {
        return true;
    } else {
        return false;
    }
}

std::vector<jl::value::Variable> jl::ir::Binary::uses() const
{
    return { m_operand_a, m_operand_b };
}

void jl::ir::Binary::replace(value::Variable from, value::Variable to)
{
    if (m_operand_a == from)
        m_operand_a = to;
    if (m_operand_b == from)
        m_operand_b = to;
    if (m_dest == from)
        m_dest = to;
}

std::optional<jl::value::Variable> jl::ir::Binary::def()
{
    return m_dest;
}