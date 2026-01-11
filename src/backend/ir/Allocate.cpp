#include "Allocate.hpp"

#include <cstdint>
#include <format>
#include <string>

jl::ir::Allocate::Allocate(value::Variable fat_ptr, value::Variable list, uint32_t elem_size, uint32_t elem_count, uint32_t line)
    : IR(line)
    , m_fat_ptr(fat_ptr)
    , m_list(list)
    , m_elem_size(elem_size)
    , m_elem_count(elem_count)
{
}

std::string jl::ir::Allocate::to_str() const
{
    return std::format("alloc {} * {}", m_elem_size, m_elem_count);
}

void jl::ir::Allocate::accept(IRVisitor& visitor)
{
    visitor.visit_allocate_ir(*this);
}

void jl::ir::Allocate::set_data(void* ptr, uint32_t size)
{
    auto bytes = static_cast<uint8_t*>(ptr);
    m_data.insert(m_data.end(), bytes, bytes + size);
}
