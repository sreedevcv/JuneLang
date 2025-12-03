#include "TypeCast.hpp"

#include <utility>

jl::ir::TypeCast::TypeCast(
    std::unique_ptr<type::Type> from,
    std::unique_ptr<type::Type> to,
    std::shared_ptr<value::Variable> dest,
    std::shared_ptr<value::Variable> source,
    uint32_t line)
    : IR(line)
    , m_dest(std::move(dest))
    , m_source(std::move(source))
    , m_from(std::move(from))
    , m_to(std::move(to))
{
}

std::string jl::ir::TypeCast::to_str() const
{
    return "cast " + m_from.get()->to_str() + " to " + m_to->to_str();
}
