#pragma once

#include "Value.hpp"
#include <cstdint>
#include <string>
#include <unordered_map>

namespace jl {
namespace value {
    class VarData {
    public:
        uint32_t add_variable(uint32_t size);

        uint32_t new_label();

        uint32_t get_size(uint32_t idx) const;

        void reset_offset(uint32_t idx, uint32_t source_idx);

        uint32_t total_size() const;

        struct Data {
            uint32_t size;
            uint32_t offset;
        };

        const std::unordered_map<uint32_t, Data>& get_offset_map() const;

    private:
        uint32_t m_var_count { 0 };
        uint32_t m_total_size { 0 };
        uint32_t m_label_count { 0 };
        std::unordered_map<uint32_t, Data> m_var_offset;
    };

    class Variable : Value {
    public:
        enum Storage {
            TEMP,
            STACK,
        };

        Variable(std::string func_id, uint32_t id, Storage storage);

        std::string to_str() const;

        uint32_t id() const;

    private:
        uint32_t m_id;
        std::string m_func_id;
        Storage m_storage;
    };
}
}
