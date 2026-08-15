#include "AllocateList.hpp"
#include "Utils.hpp"

#include <cstdint>
#include <format>
#include <optional>
#include <string>

jl::ir::AllocateList::AllocateList(value::Variable fat_ptr, value::Variable list, uint32_t elem_size, uint32_t elem_count, uint32_t line)
    : IR(line)
    , m_fat_ptr(fat_ptr)
    , m_list(list)
    , m_elem_size(elem_size)
    , m_elem_count(elem_count)
{
}

std::string jl::ir::AllocateList::to_str() const
{
    return std::format("alloc {} * {}", m_elem_size, m_elem_count);
}

void jl::ir::AllocateList::accept(IRVisitor& visitor)
{
    visitor.visit_allocate_list_ir(*this);
}

void jl::ir::AllocateList::set_data(void* ptr, uint32_t size)
{
    auto bytes = static_cast<uint8_t*>(ptr);
    m_data.insert(m_data.end(), bytes, bytes + size);
}

bool jl::ir::AllocateList::is_used(value::Variable var)
{
    if (m_fat_ptr.id() == var.id() || m_list.id() == var.id()) {
        return true;
    } else {
        return false;
    }
}

std::vector<jl::value::Variable> jl::ir::AllocateList::uses() const
{
    return { m_fat_ptr, m_list };
}

void jl::ir::AllocateList::replace(value::Variable from, value::Variable to)
{
    if (m_fat_ptr == from)
        m_fat_ptr = to;

    if (m_list == from)
        m_list = to;
}

std::optional<jl::value::Variable> jl::ir::AllocateList::def()
{
    unimplemented();
    return std::nullopt;
}
