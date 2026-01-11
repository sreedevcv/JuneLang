#include "InitLiteral.hpp"

jl::ir::InitLiteral::InitLiteral(
    std::unique_ptr<LiteralValue> source,
    value::Variable dest,
    uint32_t line)
    : IR(line)
    , m_source(std::move(source))
    , m_dest(std::move(dest))
{
}

std::string jl::ir::InitLiteral::to_str() const
{
    return m_dest.to_str() + " = " + m_source->to_str();
}

void jl::ir::InitLiteral::accept(IRVisitor& visitor)
{
    visitor.visit_init_literal_ir(*this);
}
