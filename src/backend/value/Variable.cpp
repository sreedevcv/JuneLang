#include "Variable.hpp"
#include "Utils.hpp"
#include <cstdint>

jl::value::Variable::Variable(std::string func_id, uint32_t id, Storage storage)
    : m_id(id)
    , m_func_id(func_id)
    , m_storage(storage)
{
}

std::string jl::value::Variable::to_str() const
{
    return (m_storage == TEMP ? "t" : "s") + std::to_string(m_id);
}

uint32_t jl::value::Variable::id() const
{
    return m_id;
}

jl::value::Variable::Storage jl::value::Variable::storage() const
{
    return m_storage;
}

bool jl::value::Variable::operator==(const Variable& other) const
{
    return this->m_id == other.m_id; // Compare unique identifiers
}


uint32_t jl::value::VarData::add_variable(uint32_t size)
{
    auto idx = m_var_count++;
    m_var_offset.insert({ idx, { size, m_total_size } });
    m_total_size += size;
    return idx;
}

uint32_t jl::value::VarData::new_label()
{
    return m_label_count++;
}

uint32_t jl::value::VarData::get_size(uint32_t idx) const
{
    if (idx + 1 > m_var_count) {
        unimplemented("Invalid variable index");
        return 0;
    }

    return m_var_offset.at(idx).size;
}

void jl::value::VarData::reset_offset(uint32_t idx, uint32_t source_idx)
{
    if (m_var_offset[idx].size == 0) {
        const auto actual_size = m_var_offset[source_idx].size;
        m_var_offset[idx].size = actual_size;
        m_var_offset[idx].offset = m_total_size;
        m_total_size += actual_size;
    }
}

const std::unordered_map<uint32_t, jl::value::VarData::Data>& jl::value::VarData::get_offset_map() const
{
    return m_var_offset;
}

uint32_t jl::value::VarData::total_size() const
{
    return m_total_size;
}
