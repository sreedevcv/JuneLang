#include "MachineBlock.hpp"
#include <sstream>

jl::x86::MachineBlock::MachineBlock(const std::string& name)
    : m_name(name)
{
}

std::string jl::x86::MachineBlock::to_string() const
{
    std::stringstream ss;
    ss << m_name << ": \n";

    for (const auto& instr : m_instructions) {
        ss << '\t' << instr->to_string() << '\n';
    }

    return ss.str();
}