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
    auto s = m_dest.to_str() + " = " + "phi ";
    for (const auto& [var, blk] : m_opers) {
        s += "[" + var.to_str() + " " + blk->get_name() + "] ";
    }
    return s;
}

void jl::ir::Phi::accept(IRVisitor& visitor)
{
    visitor.visit_phi(*this);
}

bool jl::ir::Phi::is_used(value::Variable variable)
{
    for (auto& [var, _] : m_opers) {
        if (var == variable) {
            return true;
        }
    }

    return false;
}

std::vector<jl::value::Variable> jl::ir::Phi::uses() const
{
    std::vector<value::Variable> result;
    for (const auto& [var, _] : m_opers) {
        result.push_back(var);
    }
    return result;
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

std::optional<jl::value::Variable> jl::ir::Phi::def()
{
    return m_dest;
}
