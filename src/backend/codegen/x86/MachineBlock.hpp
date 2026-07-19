#pragma once

#include "Instruction.hpp"
#include <list>
#include <memory>
#include <string>

namespace jl {
namespace x86 {
    class MachineBlock {
    public:
        MachineBlock(const std::string& name);

        std::string to_string() const;

        std::string m_name;
        std::list<std::unique_ptr<Instruction>> m_instructions;
    };
}
}