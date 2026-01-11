#pragma once

#include "ir/IR.hpp"
#include "value/Variable.hpp"
#include <cstdint>
#include <vector>

namespace jl {
namespace ir {
    struct Allocate : public IR {
        Allocate(value::Variable fat_ptr, value::Variable list, uint32_t elem_size, uint32_t elem_count, uint32_t line);

        ~Allocate() = default;

        std::string to_str() const override;

        void accept(IRVisitor& visitor) override;

        template <typename T>
        void set_value(const T& data)
        {
            uint64_t mask = 0xF; // There shouldn't be any data that we will be putting with size more that 64 bits
            for (int i = 0; i < sizeof(T); i++) {
                m_data.push_back((uint8_t)(data & mask));
                mask <<= 8;
            }
        }

        void set_data(void* ptr, uint32_t size);

        std::vector<uint8_t> m_data;
        value::Variable m_fat_ptr;
        value::Variable m_list;
        uint32_t m_elem_size;
        uint32_t m_elem_count;
    };
}
}
