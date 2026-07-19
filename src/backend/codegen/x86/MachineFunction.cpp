#include "MachineFunction.hpp"
#include "Function.hpp"
#include "codegen/x86/Register.hpp"
#include "ir/AllocateVar.hpp"
#include <cstdint>
#include <sstream>

jl::x86::MachineFunction::MachineFunction(const std::string& name, Function* function)
    : m_name(name)
{
    int32_t offset = 0;

    for (auto& block : function->blocks()) {
        for (auto ir = block->head; ir != nullptr; ir = ir->next) {
            if (auto alloca = dynamic_cast<ir::AllocateVar*>(ir)) {
                const auto alignment = alloca->m_addr.type()->alignment();

                if (offset % alignment != 0) {
                    offset = ((offset + alignment - 1) / alignment) * alignment;
                }

                m_stk_offset[alloca->m_addr.id()] = offset;
                offset += alloca->m_addr.type()->size();
            }
        }
    }

    total_stack_space = offset;
}

jl::x86::Register jl::x86::MachineFunction::get_register(value::Variable var)
{
    if (m_register_map.contains(var.id())) {
        return m_register_map[var.id()];
    }

    Register r(m_reg_count++);
    m_register_map[var.id()] = r;
    return r;
}
jl::x86::Register jl::x86::MachineFunction::new_register()
{
    //            return Register;
}

std::list<jl::x86::MachineBlock>& jl::x86::MachineFunction::blocks()
{
    return m_blocks;
}

std::string jl::x86::MachineFunction::to_string() const
{
    std::stringstream ss;
    ss << "; Function\n";
    ss << m_name << ": \n";

    for (const auto& block : m_blocks) {
        ss << block.to_string();
        ss << "\n";
    }

    ss << "\n";
    return ss.str();
}

jl::x86::Register jl::x86::MachineFunction::map_register(value::Variable var, Register reg)
{
    m_register_map[var.id()] = reg;
    return reg;
}

int32_t jl::x86::MachineFunction::get_ssa_offset(value::Variable var) const
{
    return m_stk_offset.at(var.id());
}