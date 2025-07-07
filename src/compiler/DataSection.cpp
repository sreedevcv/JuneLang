#include "DataSection.hpp"

jl::ptr_type jl::DataSection::add_data(const std::string& data)
{
    const auto offset = m_data.size();

    for (const auto c : data) {
        m_data.push_back(c);
    }

    return offset;
}

jl::ptr_type jl::DataSection::get_offset() const
{
    return m_data.size();
}

jl::ptr_type jl::DataSection::add_data(size_t size)
{
    const auto offset = m_data.size();

    for (int i = 0; i < size; i++) {
        m_data.push_back(0);
    }

    return offset;
}