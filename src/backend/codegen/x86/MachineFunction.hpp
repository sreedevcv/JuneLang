#pragma once

#include <cstdint>
#include <list>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "Function.hpp"
#include "MachineBlock.hpp"
#include "codegen/x86/Register.hpp"
#include "value/Variable.hpp"

namespace jl {
namespace x86 {
    class MachineFunction {
    public:
        MachineFunction(const std::string& name, Function* function);

        ~MachineFunction();

        MachineFunction(const MachineFunction&) = delete;
        MachineFunction& operator=(const MachineFunction&) = delete;

        MachineFunction(MachineFunction&&) noexcept = default;
        MachineFunction& operator=(MachineFunction&&) noexcept = default;

        VirtualRegister new_register(std::optional<PhysicalRegister> hint = std::nullopt);

        VirtualRegister get_register(value::Variable var);

        Register map_register(value::Variable var, VirtualRegister reg);

        MachineBlock* get_block(const std::string& name);

        std::list<std::unique_ptr<MachineBlock>>& blocks();

        std::string to_string() const;

        int32_t get_ssa_offset(value::Variable var) const;

        std::unordered_map<MachineBlock*, std::vector<MachineBlock*>> predecessors() const;

        std::vector<MachineBlock*> rpo() const;

        uint32_t total_stack_space;

    private:
        std::string m_name;
        std::list<std::unique_ptr<MachineBlock>> m_blocks;
        uint32_t m_reg_count = 0;
        std::unordered_map<uint32_t, VirtualRegister> m_register_map;
        std::unordered_map<uint32_t, int32_t> m_stk_offset;
        std::unordered_map<std::string, MachineBlock*> m_block_map;
    };
}
}