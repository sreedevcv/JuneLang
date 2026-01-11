#include "Jump.hpp"
#include <string>

jl::ir::Jump::Jump(uint32_t label, std::optional<value::Variable> condition, uint32_t line)
    : IR(line)
    , m_label(label)
    , m_condition(condition)
{
}

std::string jl::ir::Jump::to_str() const
{
    return "jump to label: " + std::to_string(m_label) + (m_condition ? " if " + m_condition.value().to_str() : "");
}

void jl::ir::Jump::accept(IRVisitor& visitor)
{
    visitor.visit_jump_ir(*this);

}

jl::ir::Label::Label(uint32_t value, uint32_t line)
	: IR(line)
	, m_value(value)
{

}

std::string jl::ir::Label::to_str() const
{
    return "label: " + std::to_string(m_value);
}

void jl::ir::Label::accept(IRVisitor& visitor)
{
    visitor.visit_label_ir(*this);
}
