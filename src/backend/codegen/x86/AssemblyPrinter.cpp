#include "Passes.hpp"

#include "Instruction.hpp"
#include "codegen/x86/Operand.hpp"
#include "codegen/x86/Register.hpp"

#include <sstream>

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

    std::string operator()(const jl::x86::MemoryLabel& mem) const
    {
        std::string s = mem.size != jl::x86::SizeDirective::NONE
            ? jl::x86::to_str(mem.size)
            : "";
        return s + "[" + mem.label + "]";
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

    std::string generate_mov(const jl::x86::VirtualRegister& left, const jl::x86::VirtualRegister& right)
    {
        if (left.is_float || right.is_float) {
            if (left.is_float && right.is_float) {
                return "movsd";
            }
        }

        return "mov";
    }

    void visit(jl::x86::Mov& inst)
    {
        out << generate_mov(inst.dest, inst.source)
            << " "
            << print_reg(inst.dest)
            << ", "
            << print_reg(inst.source);
    }

    void visit(jl::x86::Add& inst)
    {
        out << (inst.is_float ? "addsd " : "add ")
            << print_reg(inst.dest)
            << ", "
            << print_reg(inst.source);
    }

    void visit(jl::x86::Sub& inst)
    {
        out << (inst.is_float ? "subsd " : "sub ")
            << print_reg(inst.dest)
            << ", "
            << print_reg(inst.source);
    }

    void visit(jl::x86::Less& inst)
    {
        out << (inst.is_float ? "setb " : "setl ")
            << print_reg(inst.reg);
    }

    void visit(jl::x86::LessEqual& inst)
    {
        out << (inst.is_float ? "setbe " : "setle ")
            << print_reg(inst.reg);
    }

    void visit(jl::x86::GreaterEqual& inst)
    {
        out << (inst.is_float ? "setae " : "setge ")
            << print_reg(inst.reg);
    }

    void visit(jl::x86::Greater& inst)
    {
        out << (inst.is_float ? "seta " : "setg ")
            << print_reg(inst.reg);
    }

    void visit(jl::x86::NotEquals& inst)
    {
        out << "setne " << print_reg(inst.reg);
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
        out << (inst.is_float ? "ucomisd " : "cmp ")
            << print_reg(inst.dest)
            << ", "
            << print_reg(inst.source);
    }

    void visit(jl::x86::Lea& inst)
    {
        out << "lea "
            << print_reg(inst.dest)
            << ", "
            << print_reg(inst.source);
    }

    void visit(jl::x86::Call& inst)
    {
        out << "call " + inst.function_name;
        // if (inst.args.size() > 0) {
        // out << std::accumulate(std::next(inst.args.cbegin()),
        // inst.args.cend(),
        // print_reg(inst.args[0]),
        // [this](auto&& acc, auto&& s) { return acc + ", " + this->print_reg(s); });
        // }
    }
};

void jl::x86::pass::to_nasm_assembly(jl::x86::pass::AssemblyProgram& program, MachineFunction* function)
{
    for (auto [vreg, alloc] : function->m_allocations) {
        std::println("{} -> {}", vreg.to_str(), std::visit(MachineAllocPrinter(function), alloc));
    }

    std::stringstream out;
    out << "\n";
    for (const auto& data : function->data_section()) {
        out << "\t" << data.to_str() << "\n";
    }

    program.data_section.append(out.str());
    out.str("");

    out << "\n";
    for (auto& block : function->blocks()) {
        out << block->m_name << ":\n";

        for (auto& instr : block->m_instructions) {
            InstrPrinter printer(function);
            instr->accept(printer);
            out << "\t" << printer.get_str() << "\n";
        }
    }

    program.text_section.append(out.str());
}
