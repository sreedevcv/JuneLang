#pragma once

#include <ostream>
#include <vector>

#include "Operand.hpp"

namespace jl {
class DataSection {
public:
    ptr_type add_data(const std::string& data);
    ptr_type get_offset() const;
    ptr_type add_data(size_t size);

    std::optional<ptr_type> get_last_offset();
    std::ostream& disassemble(std::ostream& out);
    void* data();

private:
    std::vector<uint8_t> m_data;
    std::optional<ptr_type> m_last_offset;
};
}