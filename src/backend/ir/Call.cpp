#include "Call.hpp"
#include <numeric>
#include <string>

jl::ir::Call::Call(const std::string& name,
    std::vector<std::shared_ptr<value::Variable>> args,
    std::shared_ptr<value::Variable> dest,
    uint32_t line)
    : IR(line)
    , m_name(name)
    , m_args(std::move(args))
    , m_dest(dest)
{
}

std::string jl::ir::Call::to_str() const
{
    return m_dest->to_str()
        + " = call "
        + m_name
        + "("
        + std::accumulate(
            m_args.begin(),
            m_args.end(),
            std::string(""),
            [](auto a, auto b) {
                return a + b->to_str() + " ";
            })
        + ")";
}

void jl::ir::Call::accept(IRVisitor& visitor)
{
    visitor.visit_call_ir(*this);
}
