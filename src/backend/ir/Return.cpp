#include "Return.hpp"

#include "backend/ir/IR.hpp"

#include <utility>

jl::ir::Return::Return(std::optional<std::shared_ptr<value::Variable>> ret_val, uint32_t line)
    : IR(line)
    , m_ret_val(std::move(ret_val))
{
}

std::string jl::ir::Return::to_str() const
{
    return "ret " + (m_ret_val ? m_ret_val.value()->to_str() : "");
}

void jl::ir::Return::accept(IRVisitor& visitor)
{
    visitor.visit_return_ir(*this);
}
