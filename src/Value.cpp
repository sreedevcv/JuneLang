#include "Value.hpp"
#include "Callable.hpp"

#include <cstdlib>
#include <variant>

bool jl::is::_int(Value& ref)
{
    return ref.get().index() == 0;
}

bool jl::is::_float(Value& ref)
{
    return ref.get().index() == 1;
}

bool jl::is::_bool(Value& ref)
{
    return ref.get().index() == 2;
}

bool jl::is::_str(Value& ref)
{
    return ref.get().index() == 3;
}

bool jl::is::_callable(Value& ref)
{
    return ref.get().index() == 4;
}

bool jl::is::_obj(Value& ref)
{
    return ref.get().index() == 5;
}

bool jl::is::_list(Value& ref)
{
    return ref.get().index() == 6;
}

bool jl::is::_number(Value& ref)
{
    return _int(ref) || _float(ref);
}

bool jl::is::_null(Value& ref)
{
    return ref.get().index() == 7;
}

bool jl::is::_same(Value& ref1, Value& ref2)
{
    return ref1.get().index() == ref2.get().index();
}

jl::Type jl::get_type(jl::Value& value)
{
    switch (value.get().index()) {
    case 0:
        return Type::INT;
    case 1:
        return Type::FLOAT;
    case 2:
        return Type::BOOL;
    case 3:
        return Type::STR;
    case 4:
        return Type::CALL;
    case 5:
        return Type::OBJ;
    case 6:
        return Type::LIST;
    case 7:
        return Type::JNULL;
    default:
        std::exit(-1);
    }
}

bool jl::is::_exact_same(Value& ref1, Value& ref2)
{
    const auto idx1 = ref1.get().index();

    if (idx1 != ref2.get().index()) {
        return false;
    }

    switch (get_type(ref1)) {
    case Type::NONE:
        return true;
        break;
    case Type::INT:
        return std::get<int>(ref1.get()) == std::get<int>(ref2.get());
        break;
    case Type::FLOAT:
        return std::get<double>(ref1.get()) == std::get<double>(ref2.get());
        break;
    case Type::BOOL:
        return std::get<bool>(ref1.get()) == std::get<bool>(ref2.get());
        break;
    case Type::STR:
        return std::get<std::string>(ref1.get()) == std::get<std::string>(ref2.get());
        break;
    case Type::CALL:
        return std::get<Callable*>(ref1.get()) == std::get<Callable*>(ref2.get());
        break;
    case Type::OBJ:
        return std::get<Instance*>(ref1.get()) == std::get<Instance*>(ref2.get());
        break;
    case Type::LIST:
        return std::get<List>(ref1.get()) == std::get<List>(ref2.get());
        break;
    case Type::JNULL:
        return true;
        break;
    default:
        std::print("Unknow value type enum");
        std::exit(2);
        break;
    }
}

std::string jl::stringify(Value* value)
{
    if (is::_null(*value)) {
        return "null";
    } else if (is::_bool(*value)) {
        return std::get<bool>(value->get()) ? "true" : "false";
    } else if (is::_int(*value)) {
        return std::to_string(std::get<int>(value->get()));
    } else if (is::_float(*value)) {
        return std::to_string(std::get<double>(value->get()));
    } else if (is::_str(*value)) {
        return std::get<std::string>(value->get());
    } else if (is::_obj(*value)) {
        return std::get<Instance*>(value->get())->to_string();
    } else if (is::_callable(*value)) {
        return std::get<Callable*>(value->get())->to_string();
    } else if (is::_list(*value)) {
        std::string list = "[";
        for (auto& expr : std::get<std::vector<Expr*>>(value->get())) {
            if (dynamic_cast<Literal*>(expr)) {
                list.append(stringify(static_cast<Literal*>(expr)->m_value)); // NOTE::Problem to pass address og no-gc allocated Jlvalue here??
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
