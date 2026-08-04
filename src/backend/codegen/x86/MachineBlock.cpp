#include "MachineBlock.hpp"
#include "codegen/x86/Instruction.hpp"
#include <sstream>
#include <vector>

jl::x86::MachineBlock::MachineBlock(const std::string& name)
    : m_name(name)
{
}

std::string jl::x86::MachineBlock::to_string() const
{
    std::stringstream ss;
    ss << m_name << ": \n";

    for (const auto& instr : m_instructions) {
        ss << '\t' << instr->m_id << '\t' << instr->to_string() << '\n';
    }

    return ss.str();
}

std::vector<jl::x86::MachineBlock*> jl::x86::MachineBlock::successors() const
{
    std::vector<MachineBlock*> successors;

    for (const auto& instr : m_instructions) {
        if (auto jump = dynamic_cast<Jump*>(instr.get())) {
            successors.push_back(jump->target);
        }
    }

    return successors;
}

std::string jl::x86::MachineBlock::text() const
{
    std::stringstream ss;
    ss << m_name << ": \n";

    for (const auto& instr : m_instructions) {
        ss << '\t' << instr->to_string() << '\n';
    }

    return ss.str();
}
