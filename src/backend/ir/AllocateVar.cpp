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
