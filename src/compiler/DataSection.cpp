#include "DataSection.hpp"

jl::ptr_type jl::DataSection::add_data(const std::string& data)
{
    const auto offset = m_data.size();

    for (const auto c : data) {
        m_data.push_back(c);
    }

    return offset;
}