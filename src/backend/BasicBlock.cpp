#include "BasicBlock.hpp"

#include "ir/ConditionalJump.hpp"
#include "ir/Jump.hpp"
#include "ir/Return.hpp"
#include <format>

jl::ir::IR* jl::BasicBlock::get_terminator() const
{
    if (dynamic_cast<ir::Jump*>(tail)
        || dynamic_cast<ir::CondJump*>(tail)
        || dynamic_cast<ir::Return*>(tail)) {
        return tail;
    }

    return nullptr;
}

std::string jl::BasicBlock::get_name() const
{
    return std::format("{}.{}", idx, name);
}