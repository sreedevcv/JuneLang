#pragma once

#include "types/Type.hpp"
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>

namespace jl {
namespace value {
    class Variable {
    public:
        Variable(uint32_t id, const type::Type* type);

        std::string to_str() const;

        uint32_t id() const;

        const type::Type* type() const;

        bool operator==(const Variable& other) const;

    private:
        uint32_t m_id;
        const type::Type* m_type;
    };

    struct VariableHasher {
        std::size_t operator()(const Variable& var) const
        {
            auto hash1 = std::hash<uint32_t> {}(var.id());
            auto hash2 = std::hash<const type::Type*> {}(var.type());
            std::size_t seed = hash1;
            seed ^= hash2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            return seed;
        }
    };
}
}

// Hash implementation
namespace std {
template <>
struct hash<jl::value::Variable> {
    size_t operator()(const jl::value::Variable& var) const
    {
        return hash<uint32_t>()(var.id());
    }
};
}
