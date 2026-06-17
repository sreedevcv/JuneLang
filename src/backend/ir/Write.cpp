#include "Write.hpp"

#include <format>

jl::ir::Write::Write(
    value::Variable src,
    value::Variable base,
    std::optional<value::Variable> offset,
    uint32_t offset_multiplier,
    uint32_t size,
    uint32_t line)
    : IR(line)
    , m_src(src)
    , m_base(base)
    , m_offset(offset)
    , m_offset_multiplier(offset_multiplier)
    , m_size(size)
{
}

std::string jl::ir::Write::to_str() const
{
    // return std::format("write {} to {} + {} * {} <- {}",
    //     m_size, m_base.to_str(), m_offset.to_str(), m_offset_multiplier, m_src.to_str());

    return std::format("({})[{} x {}] size {} = {}", m_base.to_str(), m_offset ? m_offset->to_str() : "0", m_offset_multiplier, m_size, m_src.to_str());
}

void jl::ir::Write::accept(IRVisitor& visitor)
{
    visitor.visit_write_ir(*this);
}

bool jl::ir::Write::uses(value::Variable var)
{
    if (m_base.id() == var.id()
        || (m_offset && m_offset.value().id() == var.id())
        || m_src.id() == var.id()) {
        return true;
    } else {
        return false;
    }
}

void jl::ir::Write::replace(value::Variable from, value::Variable to)
{
    if (m_src == from)
        m_src = to;
    if (m_base == from)
        m_base = to;
    if (m_offset) {
        if (*m_offset == from)
            m_offset = to;
    }
}