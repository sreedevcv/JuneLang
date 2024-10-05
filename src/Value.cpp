#include "Value.hpp"
#include <utility>

jl::JlValue::JlValue(const Value& val)
    : value(val)
{
}

size_t jl::JlValue::index() const
{
    return value.index();
}

jl::JlValue::Value& jl::JlValue::get()
{
    return value;
}

jl::JlValue::JlValue(const Value&& val)
{
    value = std::move(val);
}