#include "Return.hpp"

#include "backend/ir/IR.hpp"

#include <optional>
#include <utility>

jl::ir::Return::Return(std::optional<value::Variable> ret_val, uint32_t line)
    : IR(line)
    , m_ret_val(std::move(ret_val))
{
}

std::string jl::ir::Return::to_str() const
{
    return "ret " + (m_ret_val ? m_ret_val.value().to_str() : "");
}

void jl::ir::Return::accept(IRVisitor& visitor)
{
    visitor.visit_return_ir(*this);
}

bool jl::ir::Return::uses(value::Variable var)
{
    if (m_ret_val && m_ret_val.value().id() == var.id()) {
        return true;
    } else {
        return false;
    }
}

void jl::ir::Return::replace(value::Variable from, value::Variable to)
{
    if (m_ret_val) {
        if (*m_ret_val == from)
            m_ret_val = to;
    }
}


std::optional<jl::value::Variable> jl::ir::Return::def()
{
    return std::nullopt;
}