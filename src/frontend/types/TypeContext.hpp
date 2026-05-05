#pragma once

#include "TypeInfo.hpp"
#include "types/Type.hpp"
#include <array>
#include <cstddef>
#include <memory>
#include <unordered_map>
#include <vector>

namespace jl {
class TypeContext {
public:
    TypeContext();

    const type::Builtin* create_builtin(type::Builtin builtin);
    const type::Pointer* create_pointer(type::Pointer pointer);
    const type::List* create_list(type::List list);
    const type::Func* create_function(type::Func func);
    std::optional<const type::Type*> from_type_info(const TypeInfo& type_info);

private:
    std::vector<std::unique_ptr<type::Type>> m_owner;
    std::array<const type::Builtin*, static_cast<int>(type::Builtin::PRIMITIVE_MAX)> m_primitive_map;
    std::unordered_map<const type::Type*, const type::Pointer*> m_pointer_map;

    struct PairHash {
        size_t operator()(const std::pair<const type::Type*, uint32_t>& p) const
        {
            auto h1 = std::hash<const type::Type*>()(p.first);
            auto h2 = std::hash<uint32_t>()(p.second);
            return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
        }
    };
    std::unordered_map<std::pair<const type::Type*, uint32_t>, const type::List*, PairHash> m_list_map;
    std::unordered_map<std::size_t, const type::Func*> m_func_map;

    size_t hash_function(const type::Func& func);


    // const type::Type* lookup(const type::Type*)
};
}