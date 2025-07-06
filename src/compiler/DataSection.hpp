#pragma once

#include <cstdint>
#include <vector>

#include "Operand.hpp"

namespace jl {
class DataSection {
public:
    ptr_type add_data(const std::string& data);

    template <typename T>
    T read_data(ptr_type offset)
    {
        T data = *(T*)(&m_data[offset]);
        return data;
    }

    template <typename T>
    void set_data(ptr_type offset, T& data)
    {
        *(T*)(&m_data[offset]) = data;
    }

private:
    std::vector<uint8_t> m_data;
};
}