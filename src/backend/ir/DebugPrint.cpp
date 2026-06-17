#include "DebugPrint.hpp"
#include "ir/IR.hpp"

jl::ir::DebugPrint::DebugPrint(
    value::Variable val,
    bool is_list,
    type::Builtin::Primitive primitive,
    uint32_t list_elem_size,
    uint32_t line)
    : IR(line)
    , m_val(val)
    , m_is_list(is_list)
    , m_primitive(primitive)
    , m_list_elem_size(list_elem_size)
{
}

std::string jl::ir::DebugPrint::to_str() const
{
    return "print " + m_val.to_str();
}

void jl::ir::DebugPrint::accept(IRVisitor& visitor)
{
    visitor.visit_debug_print_ir(*this);
}

bool jl::ir::DebugPrint::uses(value::Variable var)
{
    if (m_val.id() == var.id()) {
        return true;
    } else {
        return false;
    }
}

void jl::ir::DebugPrint::replace(value::Variable from, value::Variable to)
{
    if (m_val == from)
        m_val = to;
}