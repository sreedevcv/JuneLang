#include "Move.hpp"

#include "backend/ir/IR.hpp"

jl::ir::Move::Move(
    std::unique_ptr<value::Variable> source,
    std::shared_ptr<value::Variable> dest,
    uint32_t line)
    : IR(line)
    , m_source(std::move(source))
    , m_dest(std::move(dest))
{
}

std::string jl::ir::Move::to_str() const
{
    return m_dest->to_str() + " = " + m_source->to_str();
}