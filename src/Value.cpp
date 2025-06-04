#include "Value.hpp"
#include "Expr.hpp"

#include <type_traits>
#include <utility>
#include "Callable.hpp"

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

jl::JlList::JlList(std::vector<Expr*>& val)
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

jl::JlValue* jl::JlValue::to()
{
    return static_cast<JlValue*>(this);
}

std::string jl::to_string(JlValue* value)
{
    if (is::_null(value)) {
        return "null";
    } else if (is::_bool(value)) {
        return jl::vget<bool>(value) ? "true" : "false";
    } else if (is::_int(value)) {
        return std::to_string(jl::vget<int>(value));
    } else if (is::_float(value)) {
        return std::to_string(jl::vget<double>(value));
    } else if (is::_str(value)) {
        return jl::vget<std::string>(value);
    } else if (is::_obj(value)) {
        return jl::vget<Instance*>(value)->to_string();
    } else if (is::_callable(value)) {
        return jl::vget<Callable*>(value)->to_string();
    } else if (is::_list(value)) {
        std::string list = "[";
        for (auto expr : jl::vget<std::vector<Expr*>&>(value)) {
            if (dynamic_cast<Literal*>(expr)) {
                list.append(to_string(static_cast<Literal*>(expr)->m_value));  //NOTE::Problem to pass address og no-gc allocated Jlvalue here??
            } else {
                list.append("`expr`");
            }
            list.append(", ");
        }
        list.append("]");
        return list;
    }

    return "`null`";
}
