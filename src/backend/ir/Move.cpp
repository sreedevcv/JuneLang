#include "Move.hpp"

#include "backend/ir/IR.hpp"
#include <memory>

jl::ir::Move::Move(
    value::Variable source,
    value::Variable dest,
    uint32_t line)
    : IR(line)
    , m_source(std::move(source))
    , m_dest(std::move(dest))
{
}

std::string jl::ir::Move::to_str() const
{
    return m_dest.to_str() + " = " + m_source.to_str();
}

void jl::ir::Move::accept(IRVisitor& visitor)
{
    visitor.visit_move_ir(*this);
}
