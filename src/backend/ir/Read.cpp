#include "Read.hpp"

#include <format>

jl::ir::Read::Read(
    value::Variable dest,
    value::Variable base,
    value::Variable offset,
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
    return std::format("read {} from {} + {} * {} -> {}",
        m_size, m_base.to_str(), m_offset.to_str(), m_offset_multiplier, m_dest.to_str());
}

void jl::ir::Read::accept(IRVisitor& visitor)
{
    visitor.visit_read_ir(*this);
}
