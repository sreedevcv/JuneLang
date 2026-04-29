#include "TypeCast.hpp"

#include <utility>

jl::ir::TypeCast::TypeCast(
    const type::Type* from,
    const type::Type* to,
    value::Variable dest,
    value::Variable source,
    uint32_t line)
    : IR(line)
    , m_dest(std::move(dest))
    , m_source(std::move(source))
    , m_from(from)
    , m_to(to)
{
}

std::string jl::ir::TypeCast::to_str() const
{
    return m_dest.to_str() + " = (" + m_from->to_str() + " to " + m_to->to_str() + ") " + m_source.to_str();
}

void jl::ir::TypeCast::accept(IRVisitor& visitor)
{

}
