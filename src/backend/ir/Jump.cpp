#include "Jump.hpp"
#include "BasicBlock.hpp"
#include <string>

jl::ir::Jump::Jump(BasicBlock* target, uint32_t line)
    : IR(line)
    , m_target(target)
{
}

std::string jl::ir::Jump::to_str() const
{
    return "jump " + m_target->get_name();
}

void jl::ir::Jump::accept(IRVisitor& visitor)
{
    visitor.visit_jump_ir(*this);
}

bool jl::ir::Jump::is_used(value::Variable var)
{
    return false;
}

std::vector<jl::value::Variable> jl::ir::Jump::uses() const
{
    return {};
}

void jl::ir::Jump::replace(value::Variable from, value::Variable to)
{
}

std::optional<jl::value::Variable> jl::ir::Jump::def()
{
    return std::nullopt;
}