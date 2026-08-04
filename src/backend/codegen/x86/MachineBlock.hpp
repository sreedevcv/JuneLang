#pragma once

#include <list>
#include <memory>
#include <string>
#include <vector>

namespace jl {
namespace x86 {
    struct Instruction;

    class MachineBlock {
    public:
        MachineBlock(const std::string& name);

        std::string to_string() const;

        std::vector<MachineBlock*> successors() const;

        std::string text() const;

        std::string m_name;
        std::list<std::unique_ptr<Instruction>> m_instructions;
    };
}
}
