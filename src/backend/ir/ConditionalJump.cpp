#include "ConditionalJump.hpp"
#include "ir/IR.hpp"

#include "BasicBlock.hpp"

jl::ir::CondJump::CondJump(value::Variable condition,
    BasicBlock* true_target,
    BasicBlock* false_target,
    uint32_t line)
    : IR(line)
    , m_condition(condition)
    , m_true_target(true_target)
    , m_false_target(false_target)
{
}

std::string jl::ir::CondJump::to_str() const
{
    return "jump " + m_condition.to_str()
        + " ? " + m_true_target->get_name()
        + " : " + m_false_target->get_name();
}

void jl::ir::CondJump::accept(IRVisitor& visitor)
{
    visitor.visit_cond_jump_ir(*this);
}

bool jl::ir::CondJump::uses(value::Variable var)
{
    if (m_condition.id() == var.id()) {
        return true;
    } else {
        return false;
    }
}

void jl::ir::CondJump::replace(value::Variable from, value::Variable to)
{
    if (m_condition == from)
        m_condition = to;
}

std::optional<jl::value::Variable> jl::ir::CondJump::def()
{
    return std::nullopt;
}