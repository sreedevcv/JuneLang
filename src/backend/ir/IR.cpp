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

void jl::ir::IR::replace(IR* ir)
{
    ir->prev = prev;
    ir->next = next;
    ir->parent = parent;

    if (prev != nullptr) {
        prev->next = ir;
        prev = nullptr;
    }

    if (next != nullptr) {
        next->prev = ir;
        next = nullptr;
    }
}
