#include "Phi.hpp"
#include "ir/IR.hpp"

jl::ir::Phi::Phi(value::Variable dest, value::Variable replacing_addr)
    : IR(0)
    , m_dest(dest)
    , m_replacing_addr(replacing_addr)
{
}

std::string jl::ir::Phi::to_str() const
{
    return "phi";
}

void jl::ir::Phi::accept(IRVisitor& visitor)
{
    visitor.visit_phi(*this);
}

bool jl::ir::Phi::uses(value::Variable variable)
{
    return false;
}

void jl::ir::Phi::replace(value::Variable from, value::Variable to)
{
    if (m_dest == from)
        m_dest = to;
    if (m_replacing_addr == from)
        m_replacing_addr = to;

    for (auto& [var, _] : m_opers) {
        if (var == from)
            m_dest = to;
    }
}