#include "Variable.hpp"
// #include "Utils.hpp"
#include <cstdint>

jl::value::Variable::Variable(uint32_t id, const type::Type* type)
    : m_id(id)
    , m_type(type)
{
}

std::string jl::value::Variable::to_str() const
{
    return "v" + std::to_string(m_id) + ": " + m_type->to_str();
}

uint32_t jl::value::Variable::id() const
{
    return m_id;
}

const jl::type::Type* jl::value::Variable::type() const
{
    return m_type;
}

bool jl::value::Variable::operator==(const Variable& other) const
{
    return this->m_id == other.m_id && this->m_type == other.m_type;
}

// uint32_t jl::value::VarData::add_variable(uint32_t size)
// {
//     auto idx = m_var_count++;
//     m_var_offset.insert({ idx, { size, m_total_size, nullptr } });
//     m_total_size += size;
//     return idx;
// }

// uint32_t jl::value::VarData::add_variable(const type::Type* type)
// {
//     auto idx = m_var_count++;
//     m_var_offset.insert({ idx, { type->size(), m_total_size, type } });
//     m_total_size += type->size();
//     return idx;
// }

// uint32_t jl::value::VarData::new_label()
// {
//     return m_label_count++;
// }

// uint32_t jl::value::VarData::get_size(uint32_t idx) const
// {
//     if (idx + 1 > m_var_count) {
//         unimplemented("Invalid variable index");
//         return 0;
//     }

//     return m_var_offset.at(idx).size;
// }

// void jl::value::VarData::reset_offset(uint32_t idx, uint32_t source_idx)
// {
//     if (m_var_offset[idx].size == 0) {
//         const auto actual_size = m_var_offset[source_idx].size;
//         m_var_offset[idx].size = actual_size;
//         m_var_offset[idx].offset = m_total_size;
//         m_total_size += actual_size;
//     }
// }

// const std::unordered_map<uint32_t, jl::value::VarData::Data>& jl::value::VarData::get_offset_map() const
// {
//     return m_var_offset;
// }

// uint32_t jl::value::VarData::total_size() const
// {
//     return m_total_size;
// }
