#include "Type.hpp"
#include "Utils.hpp"
#include <memory>
#include <optional>
#include <utility>

jl::type::Type::Type(Kind kind)
    : m_kind(kind)
{
}

jl::type::Builtin::Builtin(Primitive primitive)
    : Type(Kind::BUILTIN)
    , m_primitive(primitive)
{
}

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

jl::type::Func::Func(std::unique_ptr<Type> out, std::vector<std::unique_ptr<Type>> in)
    : Type(Kind::FUNC)
    , m_out(std::move(out))
    , m_in(std::move(in))
{
}

bool jl::type::Func::equals(const Type* type) const
{
    if (type->m_kind != Kind::FUNC)
        return false;
    auto func = static_cast<const Func*>(type);
    if (func->m_in.size() != m_in.size())
        return false;

    for (auto i = 0; i < m_in.size(); i++) {
        if (!func->m_in[i]->equals(m_in[i].get())) {
            return false;
        }
    }

    return m_out->equals(func->m_out.get());
}

std::string jl::type::Func::to_str() const
{
    return "Func";
}

std::unique_ptr<jl::type::Type> jl::type::Func::clone() const
{
    std::vector<std::unique_ptr<Type>> in;

    for (const auto& type : m_in) {
        in.push_back(std::unique_ptr<Type>(type->clone()));
    }

    return std::make_unique<Func>(std::unique_ptr<Type>(m_out.get()->clone()), std::move(in));
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
    // TODO::For arrays create a new Type
    if (type_info.name == "int") {
        return std::make_unique<Builtin>(Builtin::INT);
    } else if (type_info.name == "float") {
        return std::make_unique<Builtin>(Builtin::FLOAT);
    } else if (type_info.name == "bool") {
        return std::make_unique<Builtin>(Builtin::BOOL);
    } else if (type_info.name == "char") {
        return std::make_unique<Builtin>(Builtin::CHAR);
    }

    return std::nullopt;
}

bool jl::type::is_boolean(const jl::type::Type* t)
{
    if (t->m_kind != type::Type::BUILTIN) {
        return false;
    } else {
        return static_cast<const type::Builtin*>(t)->m_primitive == type::Builtin::BOOL;
    }
}