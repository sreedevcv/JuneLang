#include "TypeContext.hpp"

#include "types/Type.hpp"

#include <cstddef>
#include <functional>
#include <memory>
#include <utility>

jl::TypeContext::TypeContext()
{
    m_owner.push_back(std::make_unique<type::Builtin>(type::Builtin::INT));
    m_primitive_map[type::Builtin::INT] = static_cast<const type::Builtin*>(m_owner.back().get());
    m_owner.push_back(std::make_unique<type::Builtin>(type::Builtin::FLOAT));
    m_primitive_map[type::Builtin::FLOAT] = static_cast<const type::Builtin*>(m_owner.back().get());
    m_owner.push_back(std::make_unique<type::Builtin>(type::Builtin::BOOL));
    m_primitive_map[type::Builtin::BOOL] = static_cast<const type::Builtin*>(m_owner.back().get());
    m_owner.push_back(std::make_unique<type::Builtin>(type::Builtin::CHAR));
    m_primitive_map[type::Builtin::CHAR] = static_cast<const type::Builtin*>(m_owner.back().get());
    m_owner.push_back(std::make_unique<type::Builtin>(type::Builtin::VOID));
    m_primitive_map[type::Builtin::VOID] = static_cast<const type::Builtin*>(m_owner.back().get());
}

const jl::type::Builtin* jl::TypeContext::create_builtin(type::Builtin builtin)
{
    const int index = static_cast<int>(builtin.m_primitive);

    if (m_primitive_map[index] != nullptr) {
        return m_primitive_map[index];
    }

    auto type = std::make_unique<type::Builtin>(builtin);
    m_primitive_map[index] = type.get();
    m_owner.push_back(std::move(type));
    return m_primitive_map[index];
}

const jl::type::Pointer* jl::TypeContext::create_pointer(type::Pointer pointer)
{
    if (m_pointer_map.contains(pointer.m_pointee)) {
        return m_pointer_map.at(pointer.m_pointee);
    }

    auto type = std::make_unique<type::Pointer>(pointer);
    m_pointer_map[pointer.m_pointee] = type.get();
    m_owner.push_back(std::move(type));
    return m_pointer_map[pointer.m_pointee];
}

const jl::type::List* jl::TypeContext::create_list(type::List list)
{
    const auto key = std::pair { list.m_elem_type, list.m_count };

    if (m_list_map.contains(key)) {
        return m_list_map[key];
    }

    auto type = std::make_unique<type::List>(list);
    m_list_map[key] = type.get();
    m_owner.push_back(std::move(type));
    return m_list_map[key];
}

const jl::type::Func* jl::TypeContext::create_function(type::Func func)
{
    const size_t hash = hash_function(func);
    if (m_func_map.contains(hash)) {
        return m_func_map[hash];
    }

    auto type = std::make_unique<type::Func>(std::move(func));
    m_func_map[hash] = type.get();
    m_owner.push_back(std::move(type));
    return m_func_map[hash];
}

size_t jl::TypeContext::hash_function(const type::Func& func)
{
    size_t hash = std::hash<const type::Type*>()(func.m_return_type);

    for (const type::Type* param : func.m_param_types) {
        hash ^= std::hash<const type::Type*>()(param)
            + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    }

    return hash;
}

std::optional<const jl::type::Type*> jl::TypeContext::from_type_info(const TypeInfo& type_info)
{
    const type::Type* t;
    if (type_info.name == "int") {
        t = create_builtin(type::Builtin(type::Builtin::INT));
    } else if (type_info.name == "float") {
        t = create_builtin(type::Builtin(type::Builtin::FLOAT));
    } else if (type_info.name == "bool") {
        t = create_builtin(type::Builtin(type::Builtin::BOOL));
    } else if (type_info.name == "char") {
        t = create_builtin(type::Builtin(type::Builtin::CHAR));
    } else {
        return std::nullopt;
    }

    if (type_info.is_array) {
        return create_list(type::List(t, type_info.size.value_or(0)));
    } else {
        return t;
    }
}
