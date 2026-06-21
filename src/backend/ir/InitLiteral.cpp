#include "InitLiteral.hpp"

jl::ir::InitLiteral::InitLiteral(
    LiteralValue source,
    value::Variable dest,
    uint32_t line)
    : IR(line)
    , m_source(source)
    , m_dest(std::move(dest))
{
}

std::string jl::ir::InitLiteral::to_str() const
{
    return m_dest.to_str() + " = " + m_source.to_str();
}

void jl::ir::InitLiteral::accept(IRVisitor& visitor)
{
    visitor.visit_init_literal_ir(*this);
}

bool jl::ir::InitLiteral::uses(value::Variable var)
{
    return false;
}

void jl::ir::InitLiteral::replace(value::Variable from, value::Variable to)
{
    if (m_dest == from)
        m_dest = to;
}

std::optional<jl::value::Variable> jl::ir::InitLiteral::def()
{
    return m_dest;
}