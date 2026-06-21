#include "AllocateVar.hpp"
#include "ir/IR.hpp"
#include <format>

jl::ir::AllocateVar::AllocateVar(value::Variable addr, const type::Type* var_type, uint32_t line)
    : IR(line)
    , m_addr(addr)
    , m_var_type(var_type)
{
}

std::string jl::ir::AllocateVar::to_str() const
{
    return std::format("{} = allocatevar {}", m_addr.to_str(), m_var_type->to_str());
}

void jl::ir::AllocateVar::accept(IRVisitor& visitor)
{
    visitor.visit_allocate_var_ir(*this);
}

bool jl::ir::AllocateVar::uses(value::Variable var)
{
    return false;
}

void jl::ir::AllocateVar::replace(value::Variable from, value::Variable to)
{
    if (m_addr == from) {
        m_addr = to;
    }
}

std::optional<jl::value::Variable> jl::ir::AllocateVar::def()
{
    return std::nullopt;
}