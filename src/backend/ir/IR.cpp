#include "IR.hpp"

jl::ir::IR::IR(uint32_t line)
    : m_line(line)
{
}

uint32_t jl::ir::IR::line() const
{
    return m_line;
}
