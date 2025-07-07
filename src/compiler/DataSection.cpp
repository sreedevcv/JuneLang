#include "DataSection.hpp"

#include <iomanip>
#include <ios>

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