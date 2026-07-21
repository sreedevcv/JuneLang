
#include "codegen/x86/Instruction.hpp"
#include "codegen/x86/MachineFunction.hpp"
#include <cstdint>

void number_instructions(jl::x86::MachineFunction* function)
{
    const auto rpo = function->rpo();
    uint32_t instr_id = 0;

    for (auto block : rpo) {
        for (auto& instr : block->m_instructions) {
            instr->m_id = instr_id;
            instr_id += 1;
        }
    }
}