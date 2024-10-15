#include "Value.hpp"
#include <utility>

jl::JlValue::JlValue(const Value& val)
    : m_value(val)
{
}

size_t jl::JlValue::index() const
{
    return m_value.index();
}

jl::JlValue::Value* jl::JlValue::get()
{
    return m_value;
}

jl::JlValue::JlValue(const Value&& val)
{
    m_value = std::move(val);
}

jl::JlValue::JlValue(Value* value)
    : m_value(*value)
{
}