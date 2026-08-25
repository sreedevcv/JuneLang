#include "Passes.hpp"
#include "RegisterAllocator.hpp"
#include "Utils.hpp"
#include "codegen/x86/Instruction.hpp"
#include "codegen/x86/MachineFunction.hpp"
#include "codegen/x86/Operand.hpp"
#include "codegen/x86/Register.hpp"
#include <print>

using VRegMap = std::unordered_map<jl::x86::VirtualRegister, jl::Allocation, jl::x86::VirtualRegisterHasher>;

const jl::x86::PhysicalRegister::Type gpr_reg_map[] = {
    jl::x86::PhysicalRegister::rbx,
    jl::x86::PhysicalRegister::rcx,
    jl::x86::PhysicalRegister::rdx,
    jl::x86::PhysicalRegister::rsi,
    jl::x86::PhysicalRegister::rdi,
    jl::x86::PhysicalRegister::r8,
    jl::x86::PhysicalRegister::r9,
    jl::x86::PhysicalRegister::r10,
    jl::x86::PhysicalRegister::r11,
    jl::x86::PhysicalRegister::r12,
    jl::x86::PhysicalRegister::r13,
    jl::x86::PhysicalRegister::r14,
    jl::x86::PhysicalRegister::r15,
};

struct MachineAllocPrinter {
    jl::x86::MachineFunction* function;

    MachineAllocPrinter(jl::x86::MachineFunction* function)
        : function(function)
    {
    }

    std::string operator()(const jl::x86::PhysicalRegister& reg) const
    {
        return reg.to_str();
    }

    std::string operator()(const jl::x86::MemoryOperand& mem) const
    {
        auto base_reg = *function->get_allocation(mem.base);
        std::string addr = std::visit(MachineAllocPrinter(function), base_reg);
        auto size_dir = (mem.size ? jl::x86::to_str(*mem.size) : "");

        if (mem.index) {
            auto index_reg = *function->get_allocation(*mem.index);
            auto index_str = std::visit(MachineAllocPrinter(function), index_reg);
            addr += std::to_string(mem.scale) + " * " + index_str;
        }

        if (mem.displacement != 0) {
            addr += std::to_string(mem.displacement);
        }

        return size_dir + "[" + addr + "]";
    }

    std::string operator()(const int64_t& imm) const
    {
        return std::to_string(imm);
    }
};

struct InstrPrinter : jl::x86::InstructionVisitor {
    jl::x86::MachineFunction* function;
    std::stringstream out;

    std::string print_reg(const jl::x86::VirtualRegister& reg)
    {
        auto alloc = *function->get_allocation(reg);
        return std::visit(MachineAllocPrinter(function), alloc);
    }

    InstrPrinter(jl::x86::MachineFunction* function)
        : function(function)
    {
    }

    std::string get_str()
    {
        return out.str();
    }

    void visit(jl::x86::Mov& inst)
    {
        out << "mov "
            << print_reg(inst.dest)
            << ", "
            << print_reg(inst.source);
    }

    void visit(jl::x86::Add& inst)
    {
        out << "add "
            << print_reg(inst.dest)
            << ", "
            << print_reg(inst.source);
    }

    void visit(jl::x86::Sub& inst)
    {
        out << "sub "
            << print_reg(inst.dest)
            << ", "
            << print_reg(inst.source);
    }

    void visit(jl::x86::Less& inst)
    {
        out << "setl " << print_reg(inst.reg);
    }

    void visit(jl::x86::Equals& inst)
    {
        out << "sete " << print_reg(inst.reg);
    }

    void visit(jl::x86::Return& inst)
    {

        out << "ret";
    }

    void visit(jl::x86::Push& inst)
    {
        out << "push " << print_reg(inst.value);
    }

    void visit(jl::x86::Pop& inst)
    {
        out << "pop " << print_reg(inst.value);
    }

    void visit(jl::x86::Jump& inst)
    {
        out << "jmp " << inst.target->m_name;
    }

    void visit(jl::x86::JumpEqual& inst)
    {
        out << "je " << inst.target->m_name;
    }

    void visit(jl::x86::Cmp& inst)
    {
        out << "cmp "
            << print_reg(inst.a)
            << ", "
            << print_reg(inst.b);
    }

    void visit(jl::x86::Lea& inst)
    {
        out << "lea "
            << print_reg(inst.dest)
            << ", "
            << print_reg(inst.source);
    }
};

jl::x86::MachineAlloc to_machine_alloc(jl::Allocation alloc, jl::x86::MachineFunction* function, uint32_t size)
{
    switch (alloc.type) {
    case jl::Allocation::GPR:
        return jl::x86::PhysicalRegister(gpr_reg_map[alloc.value]);
    case jl::Allocation::FLOAT:
        unimplemented();
    case jl::Allocation::SLOT: {
        jl::x86::MemoryOperand stack_source;
        auto base_reg = function->get_physical_register(jl::x86::PhysicalRegister::rbp);
        stack_source.base = base_reg;
        stack_source.displacement = -(alloc.value + size);
        stack_source.index = std::nullopt;
        if (auto directive = jl::x86::is_simple_move(size)) {
            stack_source.size = directive;
        }
        return stack_source;
    } break;
    }
    return {};
}

void jl::x86::pass::assign_register(jl::x86::MachineFunction* function, jl::RegisterAllocator::AllocationResult allocation_result)
{
    using namespace jl;

    for (auto [var, alloc] : allocation_result.first) {
        auto vreg = function->get_register(var);
        auto machine_alloc = to_machine_alloc(alloc, function, var.type()->size());
        function->set_allocation(vreg, machine_alloc);
    }

    for (auto& block : function->blocks()) {
        std::println("{}:", block->m_name);

        for (auto& instr : block->m_instructions) {
            InstrPrinter printer(function);
            instr->accept(printer);
            std::println("\t{}", printer.get_str());
        }
    }
}
