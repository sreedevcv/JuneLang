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

void jl::BasicBlock::replace_ir(ir::IR* ir, ir::IR* new_ir)
{

    if (auto phi = dynamic_cast<ir::Phi*>(ir)) {
        std::erase(phis, phi);
        ir->parent->insert_before(ir->parent->head, new_ir);
    } else {
        ir->replace(new_ir);
    }

    if (ir == head) {
        head = new_ir;
    } else if (ir == tail) {
        tail = new_ir;
    }
}

void jl::BasicBlock::insert_before(ir::IR* ir, ir::IR* new_ir)
{
    new_ir->next = ir;
    new_ir->prev = ir->prev;

    if (ir == head) {
        head = new_ir;
        head->prev = nullptr;
    } else {
        ir->prev->next = new_ir;
    }

    ir->prev = new_ir;
}