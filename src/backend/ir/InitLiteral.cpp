#include "InitLiteral.hpp"

jl::ir::InitLiteral::InitLiteral(std::unique_ptr<LiteralValue> source,
    std::shared_ptr<value::Variable> dest,
    uint32_t line)
    : IR(line)
    , m_source(std::move(source))
    , m_dest(std::move(dest))
{
}
