#include "Type.hpp"
#include "Utils.hpp"
#include <memory>
#include <optional>
#include <string>
#include <utility>

// constexpr jl::type::Type::Type(Kind kind)
//     : m_kind(kind)
// {
// }

// constexpr jl::type::Builtin::Builtin(Primitive primitive)
//     : Type(Kind::BUILTIN)
//     , m_primitive(primitive)
// {
// }

bool jl::type::Builtin::equals(const Type* type) const
{
    if (type->m_kind != Kind::BUILTIN)
        return false;
    return static_cast<const Builtin*>(type)->m_primitive == m_primitive;
}

std::string jl::type::Builtin::to_str() const
{
    switch (m_primitive) {
    case INT:
        return "int";
    case FLOAT:
        return "float";
    case BOOL:
        return "bool";
    case CHAR:
        return "char";
    case VOID:
        return "void";
    default:
        unimplemented();
        break;
    }
    return "Builtin";
}

std::unique_ptr<jl::type::Type> jl::type::Builtin::clone() const
{
    return std::make_unique<Builtin>(m_primitive);
}

uint32_t jl::type::Builtin::size() const
{
    switch (m_primitive) {
    case INT:
        return 8;
    case FLOAT:
        return 8;
    case BOOL:
        return 1; // Change this back to 1 or 4
    case CHAR:
        return 1;
    case VOID:
        return 0;
    default:
        unimplemented();
        break;
    }
    return 0;
}

jl::type::Pointer::Pointer(std::unique_ptr<Type> pointee)
    : Type(Kind::PTR)
    , m_pointee(std::move(pointee))
{
}

bool jl::type::Pointer::equals(const Type* type) const
{
    if (type->m_kind != Kind::PTR)
        return false;
    return static_cast<const Pointer*>(type)->m_pointee->equals(this->m_pointee.get());
}

std::string jl::type::Pointer::to_str() const
{
    return "Pointer";
}

std::unique_ptr<jl::type::Type> jl::type::Pointer::clone() const
{
    return std::make_unique<Pointer>(std::unique_ptr<Type>(m_pointee.get()->clone()));
}

uint32_t jl::type::Pointer::size() const
{
    return 8;
}

jl::type::Func::Func(std::unique_ptr<Type> return_type, std::vector<std::unique_ptr<Type>> param_types)
    : Type(Kind::FUNC)
    , m_return_type(std::move(return_type))
    , m_param_types(std::move(param_types))
{
}

bool jl::type::Func::equals(const Type* type) const
{
    if (type->m_kind != Kind::FUNC)
        return false;

    auto func = static_cast<const Func*>(type);

    if (func->m_param_types.size() != m_param_types.size())
        return false;

    for (auto i = 0; i < m_param_types.size(); i++) {
        if (!func->m_param_types[i]->equals(m_param_types[i].get())) {
            return false;
        }
    }

    return m_return_type.get()->equals(func->m_return_type.get());
}

std::string jl::type::Func::to_str() const
{
    std::string str = "Fun(";
    for (int i = 0; i < static_cast<int>(m_param_types.size()) - 1; i++) {
        str += m_param_types[i]->to_str() + ", ";
    }

    if (m_param_types.size() > 0) {
        str += m_param_types.back()->to_str();
    }

    str += ") -> " + m_return_type.get()->to_str();

    return str;
}

std::unique_ptr<jl::type::Type> jl::type::Func::clone() const
{
    std::vector<std::unique_ptr<Type>> in;

    for (const auto& type : m_param_types) {
        in.push_back(std::unique_ptr<Type>(type->clone()));
    }

    return std::make_unique<Func>(m_return_type.get()->clone(), std::move(in));
}

uint32_t jl::type::Func::size() const
{
    return 0;
}

jl::type::List::List(std::unique_ptr<Type> elem_type, uint32_t count)
    : Type(LIST)
    , m_elem_type(std::move(elem_type))
    , m_count(count)
{
}

bool jl::type::List::equals(const Type* type) const
{
    if (type->m_kind != LIST) {
        return false;
    }

    auto list = static_cast<const List*>(type);
    return list->m_elem_type->equals(m_elem_type.get());
}

std::string jl::type::List::to_str() const
{
    return "[" + m_elem_type->to_str() + "; " + std::to_string(m_count) + "]";
}

std::unique_ptr<jl::type::Type> jl::type::List::clone() const
{
    return std::make_unique<List>(m_elem_type->clone(), m_count);
}

uint32_t jl::type::List::size() const
{
    return 16;
}

bool jl::type::is_number(const Type* t)
{
    if (t->m_kind == Type::BUILTIN) {
        auto a = static_cast<const Builtin*>(t);
        return a->m_primitive == Builtin::INT || a->m_primitive == Builtin::FLOAT;
    }

    return false;
}

std::optional<std::unique_ptr<jl::type::Type>> jl::type::from_type_info(const TypeInfo& type_info)
{
    std::unique_ptr<Type> t;
    if (type_info.name == "int") {
        t = std::make_unique<Builtin>(Builtin::INT);
    } else if (type_info.name == "float") {
        t = std::make_unique<Builtin>(Builtin::FLOAT);
    } else if (type_info.name == "bool") {
        t = std::make_unique<Builtin>(Builtin::BOOL);
    } else if (type_info.name == "char") {
        t = std::make_unique<Builtin>(Builtin::CHAR);
    } else {
        return std::nullopt;
    }

    if (type_info.is_array) {
        return std::make_unique<List>(std::move(t), type_info.size.value_or(0));
    } else {
        return t;
    }
}

bool jl::type::is_boolean(const jl::type::Type* t)
{
    if (t->m_kind != type::Type::BUILTIN) {
        return false;
    } else {
        return static_cast<const type::Builtin*>(t)->m_primitive == type::Builtin::BOOL;
    }
}
