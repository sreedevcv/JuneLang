#include "DataSection.hpp"

#include <iomanip>
#include <ios>
#include <optional>

jl::ptr_type jl::DataSection::add_data(const std::string& data)
{
    const auto offset = m_data.size();
    m_last_offset = offset;

    for (const auto c : data) {
        m_data.push_back(c);
    }

    m_data.push_back('\0');

    return offset;
}

jl::ptr_type jl::DataSection::get_offset() const
{
    return m_data.size();
}

jl::ptr_type jl::DataSection::add_data(size_t size)
{
    const auto offset = m_data.size();
    m_last_offset = offset;

    for (int i = 0; i < size; i++) {
        m_data.push_back(0);
    }

    return offset;
}

std::ostream& jl::DataSection::disassemble(std::ostream& out)
{
    int length = 32;
    int start = 0;
    int i;

    while (start < m_data.size()) {
        out << std::hex << std::setfill('0') << std::setw(4) << start << ' ';

        for (i = start; i < m_data.size() && i < start + length; i++) {
            out << std::hex << std::setw(2) << (int)m_data[i];
        }
        while (i < start + length) {
            out << std::hex << std::setw(2) << (int)0;
            i++;
        }

        out << "\t|\t";

        for (i = start; i < m_data.size() && i < start + length; i++) {
            out << m_data[i];
        }

        while (i < start + length) {
            out << ' ';
            i++;
        }

        start += length;
        out << '\n';
    }

    return out;
}

void* jl::DataSection::data()
{
    return m_data.data();
}

std::optional<jl::ptr_type> jl::DataSection::get_last_offset()
{
    const auto ret = m_last_offset;

    if (m_last_offset) {
        m_last_offset = std::nullopt;
    }

    return ret;
}