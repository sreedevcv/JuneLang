#include "BasicBlock.hpp"

#include "ir/ConditionalJump.hpp"
#include "ir/Jump.hpp"
#include "ir/Phi.hpp"
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

void jl::BasicBlock::remove_ir(ir::IR* ir)
{
    if (ir == head) {
        head = head->next;
    } else if (ir == tail) {
        tail = tail->prev;
    }

    if (auto phi = dynamic_cast<ir::Phi*>(ir)) {
        for (int i = 0; i < phis.size(); i++) {
            if (phis[i] == ir) {
                phis.erase(phis.begin() + i);
            }
        }
    }

    ir->remove();
}