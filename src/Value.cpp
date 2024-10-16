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

jl::JlInt::JlInt(int val)
    : m_val(val)
{
    m_type = INT;
}

jl::JlFloat::JlFloat(double val)
    : m_val(val)
{
    m_type = FLOAT;
}

jl::JlBool::JlBool(bool val)
    : m_val(val)
{
    m_type = BOOL;
}

jl::JlStr::JlStr(const std::string& val)
    : m_val(val)
{
    m_type = STR;
}

jl::JlCallable::JlCallable(Callable* val)
    : m_val(val)
{
    m_type = CALL;
}

jl::JlObj::JlObj(Instance* val)
    : m_val(val)
{
    m_type = OBJ;
}

jl::JlList::JlList(std::vector<Expr*>* val)
    : m_val(val)
{
    m_type = LIST;
}

jl::JlNull::JlNull()
{
    m_type = JNULL;
}

bool jl::is::_int(JlValue* ref)
{
    return dynamic_cast<JlInt*>(ref) != nullptr;
}

bool jl::is::_float(JlValue* ref)
{
    return dynamic_cast<JlFloat*>(ref) != nullptr;
}

bool jl::is::_bool(JlValue* ref)
{
    return dynamic_cast<JlBool*>(ref) != nullptr;
}

bool jl::is::_str(JlValue* ref)
{
    return dynamic_cast<JlStr*>(ref) != nullptr;
}

bool jl::is::_callable(JlValue* ref)
{
    return dynamic_cast<JlCallable*>(ref) != nullptr;
}

bool jl::is::_obj(JlValue* ref)
{
    return dynamic_cast<JlObj*>(ref) != nullptr;
}

bool jl::is::_list(JlValue* ref)
{
    return dynamic_cast<JlList*>(ref) != nullptr;
}

bool jl::is::_number(JlValue* ref)
{
    return _int(ref) || _float(ref);
}

bool jl::is::_null(JlValue* ref)
{
    return ref->m_type == JlValue::JNULL;
}

bool jl::is::_same(JlValue* ref1, JlValue* ref2)
{
    return ref1->m_type == ref2->m_type;
}

bool jl::is::_exact_same(JlValue* ref1, JlValue* ref2)
{
    switch (ref1->m_type) {
    case JlValue::NONE:
        return true;
        break;
    case JlValue::INT:
        return static_cast<JlInt*>(ref1)->m_val == static_cast<JlInt*>(ref2)->m_val;
        break;
    case JlValue::FLOAT:
        return static_cast<JlFloat*>(ref1)->m_val == static_cast<JlFloat*>(ref2)->m_val;
        break;
    case JlValue::BOOL:
        return static_cast<JlBool*>(ref1)->m_val == static_cast<JlBool*>(ref2)->m_val;
        break;
    case JlValue::STR:
        return static_cast<JlStr*>(ref1)->m_val == static_cast<JlStr*>(ref2)->m_val;
        break;
    case JlValue::CALL:
        return static_cast<JlCallable*>(ref1)->m_val == static_cast<JlCallable*>(ref2)->m_val;
        break;
    case JlValue::OBJ:
        return static_cast<JlObj*>(ref1)->m_val == static_cast<JlObj*>(ref2)->m_val;
        break;
    case JlValue::LIST:
        return static_cast<JlList*>(ref1)->m_val == static_cast<JlList*>(ref2)->m_val;
        break;
    case JlValue::JNULL:
        return true;
        break;
    default:
        std::print("Unknow value type enum");
        std::exit(2);
        break;
    }
}