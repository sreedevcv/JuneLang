#include "Variable.hpp"

jl::value::Variable::Variable(uint32_t id)
    : m_id(id)
{
}

std::string jl::value::Variable::to_str() const
{
    return "t" + std::to_string(m_id);
}