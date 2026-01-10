#include "DebugPrint.hpp"
#include "ir/IR.hpp"

jl::ir::DebugPrint::DebugPrint(std::shared_ptr<value::Variable> val, type::Builtin::Primitive primitive, uint32_t line)
    : IR(line)
    , m_val(val)
    , m_primitive(primitive)
{
}

std::string jl::ir::DebugPrint::to_str() const
{
    return "print " + m_val->to_str();
}

void jl::ir::DebugPrint::accept(IRVisitor& visitor)
{
    visitor.visit_debug_print_ir(*this);
}
