#pragma once

#include <cstdint>
#include <list>
#include <string>
#include <unordered_map>

#include "Function.hpp"
#include "MachineBlock.hpp"
#include "codegen/x86/Register.hpp"
#include "value/Variable.hpp"

namespace jl {
namespace x86 {
    class MachineFunction {
    public:
        MachineFunction(const std::string& name, Function* function);

        Register new_register();

        Register get_register(value::Variable var);

        Register map_register(value::Variable var, Register reg);

        std::list<MachineBlock>& blocks();

        std::string to_string() const;

        int32_t get_ssa_offset(value::Variable var) const;

        uint32_t total_stack_space;

    private:
        std::string m_name;
        std::list<MachineBlock> m_blocks;
        uint32_t m_reg_count = 0;
        std::unordered_map<uint32_t, Register> m_register_map;
        std::unordered_map<uint32_t, int32_t> m_stk_offset;
    };
}
}