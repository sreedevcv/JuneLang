#include "Call.hpp"
#include <numeric>
#include <string>

jl::ir::Call::Call(const std::string& name,
    std::vector<value::Variable> args,
    value::Variable dest,
    uint32_t line)
    : IR(line)
    , m_name(name)
    , m_args(std::move(args))
    , m_dest(dest)
{
}

std::string jl::ir::Call::to_str() const
{
    return m_dest.to_str()
        + " = call "
        + m_name
        + "("
        + std::accumulate(
            m_args.begin(),
            m_args.end(),
            std::string(""),
            [](auto a, auto b) {
                return a + b.to_str() + " ";
            })
        + ")";
}

void jl::ir::Call::accept(IRVisitor& visitor)
{
    visitor.visit_call_ir(*this);
}

bool jl::ir::Call::uses(value::Variable var)
{
    for (const auto& arg : m_args) {
        if (arg.id() == var.id()) {
            return true;
        }
    }

    if (m_dest.id() == var.id()) {
        return true;
    }

    return false;
}

void jl::ir::Call::replace(value::Variable from, value::Variable to)
{
    if (m_dest == from)
        m_dest = to;

    for (auto& arg : m_args) {
        if (arg == from)
            arg = to;
    }
}

std::optional<jl::value::Variable> jl::ir::Call::def()
{
    return m_dest;
}