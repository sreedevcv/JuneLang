#include "Variable.hpp"

jl::value::Variable::Variable(uint32_t id, Storage storage)
    : m_id(id)
    , m_storage(storage)
{
}

std::string jl::value::Variable::to_str() const
{
    return (m_storage == TEMP ? "t" : "s") + std::to_string(m_id);
}
