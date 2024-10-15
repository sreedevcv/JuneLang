#include "Value.hpp"

#include <type_traits>
#include <utility>

/*jl::JlValue::JlValue(const Value& val)
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
}*/

bool jl::is::_int(JlValue* ref)
{
    return 
}

bool jl::is::_float(JlValue* ref)
{
}

bool jl::is::_bool(JlValue* ref)
{
}

bool jl::is::_str(JlValue* ref)
{
}

bool jl::is::_callable(JlValue* ref)
{
}

bool jl::is::_obj(JlValue* ref)
{
}

bool jl::is::_list(JlValue* ref)
{
}
