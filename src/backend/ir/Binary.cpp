#include "Binary.hpp"

jl::ir::Binary::Binary(
    std::shared_ptr<value::Variable> dest,
    std::shared_ptr<value::Variable> operand_a,
    std::shared_ptr<value::Variable> operand_b,
    Operation operation,
    uint32_t line)
    : IR(line)
    , m_dest(dest)
    , m_operand_a(operand_a)
    , m_operand_b(operand_b)
    , m_operation(operation)
{
}

std::string jl::ir::Binary::to_str() const
{
    switch (m_operation) {
    case PLUS:
        return m_dest->to_str() + " = " + m_operand_a->to_str() + " + " + m_operand_b->to_str();
    case MINUS:
        return m_dest->to_str() + " = " + m_operand_a->to_str() + " - " + m_operand_b->to_str();
    case STAR:
        return m_dest->to_str() + " = " + m_operand_a->to_str() + " * " + m_operand_b->to_str();
    case SLASH:
        return m_dest->to_str() + " = " + m_operand_a->to_str() + " / " + m_operand_b->to_str();
    case GREATER:
        return m_dest->to_str() + " = " + m_operand_a->to_str() + " > " + m_operand_b->to_str();
    case LESS:
        return m_dest->to_str() + " = " + m_operand_a->to_str() + " < " + m_operand_b->to_str();
    case GREATER_EQUAL:
        return m_dest->to_str() + " = " + m_operand_a->to_str() + " >= " + m_operand_b->to_str();
    case LESS_EQUAL:
        return m_dest->to_str() + " = " + m_operand_a->to_str() + " <= " + m_operand_b->to_str();
    case EQUAL_EQUAL:
        return m_dest->to_str() + " = " + m_operand_a->to_str() + " == " + m_operand_b->to_str();
    case BANG_EQUAL:
        return m_dest->to_str() + " = " + m_operand_a->to_str() + " != " + m_operand_b->to_str();
    case PERCENT:
        return m_dest->to_str() + " = " + m_operand_a->to_str() + " % " + m_operand_b->to_str();
    case BIT_AND:
        return m_dest->to_str() + " = " + m_operand_a->to_str() + " & " + m_operand_b->to_str();
    case BIT_OR:
        return m_dest->to_str() + " = " + m_operand_a->to_str() + " | " + m_operand_b->to_str();
    case BIT_XOR:
        return m_dest->to_str() + " = " + m_operand_a->to_str() + " ^ " + m_operand_b->to_str();
    case LOG_AND:
        return m_dest->to_str() + " = " + m_operand_a->to_str() + " && " + m_operand_b->to_str();
    case LOG_OR:
        return m_dest->to_str() + " = " + m_operand_a->to_str() + " || " + m_operand_b->to_str();
    }
}
