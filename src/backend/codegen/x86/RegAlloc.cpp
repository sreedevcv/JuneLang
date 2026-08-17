#include "Passes.hpp"
#include "RegisterAllocator.hpp"
#include "codegen/x86/Instruction.hpp"
#include "codegen/x86/Register.hpp"

constexpr jl::x86::PhysicalRegister input_registers = {

};

struct AllocationAssigner : jl::x86::InstructionVisitor {
    const std::unordered_map<jl::x86::VirtualRegister, jl::Allocation, jl::x86::VirtualRegisterHasher>& allocs;

    AllocationAssigner(const std::unordered_map<jl::x86::VirtualRegister, jl::Allocation, jl::x86::VirtualRegisterHasher>& allocs)
        : allocs(allocs)
    {
    }

    // struct OperandAllocationAssigner {
    // const std::unordered_map<jl::x86::VirtualRegister, jl::Allocation, jl::x86::VirtualRegisterHasher>& allocs;
    //
    // OperandAllocationAssigner(const std::unordered_map<jl::x86::VirtualRegister, jl::Allocation, jl::x86::VirtualRegisterHasher>& allocs)
    // : allocs(allocs)
    // {
    // }
    //
    // void operator()(jl::x86::VirtualRegister& reg) {
    // reg.allocation = allocs.at(reg);
    // }
    // };

    void visit(jl::x86::Mov& inst)
    {
        if (auto reg = std::get_if<jl::x86::VirtualRegister>(&inst.source)) {
        }
    }
    void visit(jl::x86::Add& inst) { }
    void visit(jl::x86::Sub& inst) { }
    void visit(jl::x86::Less& inst) { }
    void visit(jl::x86::Equals& inst) { }
    void visit(jl::x86::Return& inst) { }
    void visit(jl::x86::Push& inst) { }
    void visit(jl::x86::Pop& inst) { }
    void visit(jl::x86::Jump& inst) { }
    void visit(jl::x86::JumpEqual& inst) { }
    void visit(jl::x86::Cmp& inst) { }
    void visit(jl::x86::Lea& inst) { }
};

void jl::x86::pass::assign_register(jl::x86::MachineFunction* function, jl::RegisterAllocator::AllocationResult allocation_result)
{
    using namespace jl;

    std::unordered_map<x86::VirtualRegister, Allocation, x86::VirtualRegisterHasher> allocs;

    for (auto [var, alloc] : allocation_result.first) {
        auto vreg = function->get_register(var);
        allocs[vreg] = alloc;
    }

    for (auto& block : function->blocks()) {
        for (auto& instr : block->m_instructions) {
        }
    }
}
