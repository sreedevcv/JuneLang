#pragma once

#include "types/Type.hpp"
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>

namespace jl {
namespace value {
    // class VarData {
    // public:
    //     uint32_t add_variable(uint32_t size);

    //     uint32_t add_variable(const type::Type* m_type);

    //     uint32_t new_label();

    //     uint32_t get_size(uint32_t idx) const;

    //     void reset_offset(uint32_t idx, uint32_t source_idx);

    //     uint32_t total_size() const;

    //     struct Data {
    //         uint32_t size;
    //         uint32_t offset;
    //         const type::Type* m_type;
    //     };

    //     const std::unordered_map<uint32_t, Data>& get_offset_map() const;

    // private:
    //     uint32_t m_var_count { 0 };
    //     uint32_t m_total_size { 0 };
    //     uint32_t m_label_count { 0 };
    //     std::unordered_map<uint32_t, Data> m_var_offset;
    // };

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
