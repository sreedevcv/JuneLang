#include "Read.hpp"

#include <format>

jl::ir::Read::Read(
    value::Variable dest,
    value::Variable base,
    std::optional<value::Variable> offset,
    uint32_t offset_multiplier,
    uint32_t size,
    uint32_t line)
    : IR(line)
    , m_dest(dest)
    , m_base(base)
    , m_offset(offset)
    , m_offset_multiplier(offset_multiplier)
    , m_size(size)
{
}

std::string jl::ir::Read::to_str() const
{
    return std::format("{} = ({})[{} x {}] size {}", m_dest.to_str(), m_base.to_str(), m_offset ? m_offset->to_str() : "0", m_offset_multiplier, m_size);
}

void jl::ir::Read::accept(IRVisitor& visitor)
{
    visitor.visit_read_ir(*this);
}
