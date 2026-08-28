#include "Passes.hpp"

#include "RegisterAllocator.hpp"
#include "Utils.hpp"
#include "codegen/x86/Instruction.hpp"
#include "codegen/x86/MachineFunction.hpp"
#include "codegen/x86/Operand.hpp"
#include "codegen/x86/Register.hpp"

#include <cassert>
#include <print>
#include <variant>

using VRegMap = std::unordered_map<jl::x86::VirtualRegister, jl::Allocation, jl::x86::VirtualRegisterHasher>;

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
        return jl::x86::PhysicalRegister(static_cast<jl::x86::PhysicalRegister::Type>(alloc.value));
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

// Move the function params from register to stack if they are allocated in stack
void move_inputs_to_stk_if_needed(jl::x86::MachineFunction* function, const jl::x86::pass::AllocationMap& allocations)
{
    std::vector<std::unique_ptr<jl::x86::Instruction>> moves;

    // Move the input arguments to the allocated regs/stacks
    for (int i = 0; i < function->inputs().size(); i++) {
        auto param = function->inputs()[i];
        const auto& alloc = allocations.at(param);
        if (alloc.type != jl::Allocation::SLOT)
            continue;

        assert(alloc.type != jl::Allocation::FLOAT);

        auto source = function->new_register();
        auto dest = function->new_register();
        function->set_allocation(source, jl::x86::PhysicalRegister(jl::x86::input_registers[i]));
        function->set_allocation(dest, *function->get_allocation(param));

        auto move = new jl::x86::Mov();
        move->source = source;
        move->dest = dest;

        moves.emplace_back(move);
    }

    auto& entry = function->blocks().front()->m_instructions;
    // std::println("before {}", entry.size());
    for (auto& mov : moves) {
        entry.insert(entry.begin(), std::move(mov));
    }
    // std::println("after {}", entry.size());
}

void rewrite_mem_to_mem_moves(jl::x86::MachineFunction* function)
{
    auto scratch = function->get_physical_register(jl::x86::PhysicalRegister::rax);

    for (auto& block : function->blocks()) {
        for (auto iter = block->m_instructions.begin(); iter != block->m_instructions.end(); ++iter) {
            auto& instr = *iter;

            if (auto mov = dynamic_cast<jl::x86::Mov*>(instr.get())) {
                const auto dest = *function->get_allocation(mov->dest);
                const auto source = *function->get_allocation(mov->source);

                if (dest.index() == source.index()) {
                    if (auto mem_src = std::get_if<jl::x86::MemoryOperand>(&source)) {
                        if (auto mem_dest = std::get_if<jl::x86::MemoryOperand>(&dest)) {
                            // insert a mov from source to scratch register
                            auto new_move = new jl::x86::Mov();
                            new_move->dest = scratch;
                            new_move->source = mov->source;
                            block->m_instructions.insert(iter, std::unique_ptr<jl::x86::Instruction> { new_move });
                            // Change the source in the existing instruction to the scratch register
                            mov->source = scratch;
                        }
                    }
                }
            }
        }
    }
}

void jl::x86::pass::assign_register(jl::x86::MachineFunction* function, AllocationMap allocations)
{
    using namespace jl;

    for (auto [vreg, alloc] : allocations) {
        auto var = *function->get_variable(vreg);
        auto machine_alloc = to_machine_alloc(alloc, function, var.type()->size());
        // std::println("{} = {} -> {}", var.to_str(), vreg.to_str(), std::visit(MachineAllocPrinter(function), machine_alloc));
        function->set_allocation(vreg, machine_alloc);
    }

    for (auto [vreg, alloc] : function->m_allocations) {
        std::println("{} -> {}", vreg.to_str(), std::visit(MachineAllocPrinter(function), alloc));
    }

    move_inputs_to_stk_if_needed(function, allocations);
    rewrite_mem_to_mem_moves(function);

    for (auto& block : function->blocks()) {
        std::println("{}:", block->m_name);

        for (auto& instr : block->m_instructions) {
            InstrPrinter printer(function);
            instr->accept(printer);
            std::println("\t{}", printer.get_str());
        }
    }
}
