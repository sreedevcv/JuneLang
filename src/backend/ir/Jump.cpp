#include "Jump.hpp"
#include <string>
#include "BasicBlock.hpp"

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
