#include "IR.hpp"

jl::ir::IR::IR(uint32_t line)
    : m_line(line)
{
}

uint32_t jl::ir::IR::line() const
{
    return m_line;
}

void jl::ir::IR::insert_before(IR* ir)
{
    if (prev != nullptr) {
        prev->next = ir;
    }
    ir->prev = prev;
    ir->next = this;
    this->prev = ir;
}

void jl::ir::IR::remove()
{
    if (prev != nullptr) {
        prev->next = next;
    }

    if (next != nullptr) {
        next->prev = prev;
    }

    prev = nullptr;
    next = nullptr;
}
