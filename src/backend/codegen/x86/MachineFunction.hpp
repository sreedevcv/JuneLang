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

        VirtualRegister map_register(value::Variable var, VirtualRegister reg);

        void map_physical_register(PhysicalRegister::Type reg);

        VirtualRegister get_physical_register(PhysicalRegister::Type reg) const;

        MachineBlock* get_block(const std::string& name);

        std::list<std::unique_ptr<MachineBlock>>& blocks();

        std::vector<VirtualRegister>& inputs();

        std::string to_string() const;

        int32_t get_ssa_offset(value::Variable var) const;

        std::unordered_map<MachineBlock*, std::vector<MachineBlock*>> predecessors() const;

        std::unordered_map<MachineBlock*, std::vector<MachineBlock*>> successors() const;

        std::vector<MachineBlock*> rpo() const;

        uint32_t total_stack_space;

    private:
        std::string m_name;
        std::list<std::unique_ptr<MachineBlock>> m_blocks;
        uint32_t m_reg_count = 0;
        std::vector<VirtualRegister> m_inputs;
        std::unordered_map<uint32_t, VirtualRegister> m_register_map;
        std::unordered_map<uint32_t, int32_t> m_stk_offset;
        std::unordered_map<std::string, MachineBlock*> m_block_map;
        std::unordered_map<PhysicalRegister::Type, VirtualRegister> m_physical_register_map;
    };
}
}
