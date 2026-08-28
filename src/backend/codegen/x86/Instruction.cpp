#include "Instruction.hpp"
#include "codegen/x86/Register.hpp"

//============================MOV=============================

std::string jl::x86::Mov::to_str() const
{
    OperandPrinter printer;
    return "mov "
        + dest.to_str()
        + ", " + source.to_str();
}

std::vector<jl::x86::VirtualRegister> jl::x86::Mov::defs() const
{
    return { dest };
}

std::vector<jl::x86::VirtualRegister> jl::x86::Mov::uses() const
{
    return { source };
}

void jl::x86::Mov::replace(jl::x86::VirtualRegister reg, jl::x86::Operand operand)
{
}

void jl::x86::Mov::accept(jl::x86::InstructionVisitor& visitor)
{
    visitor.visit(*this);
}

//============================ADD=============================

std::string jl::x86::Add::to_str() const
{
    OperandPrinter printer;
    return "add "
        + dest.to_str()
        + ", " + source.to_str();
}

std::vector<jl::x86::VirtualRegister> jl::x86::Add::defs() const
{
    return { dest };
}

std::vector<jl::x86::VirtualRegister> jl::x86::Add::uses() const
{
    return { source, dest };
}

void jl::x86::Add::replace(jl::x86::VirtualRegister reg, jl::x86::Operand operand)
{
}

void jl::x86::Add::accept(jl::x86::InstructionVisitor& visitor)
{
    visitor.visit(*this);
}

//============================SUB=============================

std::string jl::x86::Sub::to_str() const
{
    OperandPrinter printer;
    return "sub "
        + dest.to_str()
        + ", " + source.to_str();
}

std::vector<jl::x86::VirtualRegister> jl::x86::Sub::defs() const
{
    return { dest };
}

std::vector<jl::x86::VirtualRegister> jl::x86::Sub::uses() const
{
    return { source, dest };
}

void jl::x86::Sub::replace(jl::x86::VirtualRegister reg, jl::x86::Operand operand)
{
}

void jl::x86::Sub::accept(jl::x86::InstructionVisitor& visitor)
{
    visitor.visit(*this);
}

//============================LESS=============================

std::string jl::x86::Less::to_str() const
{
    return "setl " + reg.to_str();
}

std::vector<jl::x86::VirtualRegister> jl::x86::Less::defs() const
{
    return { reg };
}

std::vector<jl::x86::VirtualRegister> jl::x86::Less::uses() const
{
    return {};
}

void jl::x86::Less::replace(jl::x86::VirtualRegister r, jl::x86::Operand operand)
{
}

void jl::x86::Less::accept(jl::x86::InstructionVisitor& visitor)
{
    visitor.visit(*this);
}

//============================EQUALS=============================

std::string jl::x86::Equals::to_str() const
{
    return "sete " + reg.to_str();
}

std::vector<jl::x86::VirtualRegister> jl::x86::Equals::defs() const
{
    return { reg };
}

std::vector<jl::x86::VirtualRegister> jl::x86::Equals::uses() const
{
    return {};
}

void jl::x86::Equals::replace(jl::x86::VirtualRegister r, jl::x86::Operand operand)
{
}

void jl::x86::Equals::accept(jl::x86::InstructionVisitor& visitor)
{
    visitor.visit(*this);
}

//============================RETURN=============================

std::string jl::x86::Return::to_str() const
{
    return "ret";
}

std::vector<jl::x86::VirtualRegister> jl::x86::Return::defs() const
{
    return {};
}

std::vector<jl::x86::VirtualRegister> jl::x86::Return::uses() const
{
    return {};
}

void jl::x86::Return::replace(jl::x86::VirtualRegister, Operand) { }

void jl::x86::Return::accept(jl::x86::InstructionVisitor& visitor)
{
    visitor.visit(*this);
}

//============================PUSH=============================

std::string jl::x86::Push::to_str() const
{
    return "push " + value.to_str();
}

std::vector<jl::x86::VirtualRegister> jl::x86::Push::defs() const
{
    return {};
}

std::vector<jl::x86::VirtualRegister> jl::x86::Push::uses() const
{
    return { value };
}

void jl::x86::Push::replace(jl::x86::VirtualRegister r, jl::x86::Operand operand)
{
}

void jl::x86::Push::accept(jl::x86::InstructionVisitor& visitor)
{
}

//============================POP=============================

std::string jl::x86::Pop::to_str() const
{
    return "pop " + value.to_str();
}

std::vector<jl::x86::VirtualRegister> jl::x86::Pop::defs() const
{
    return { value };
}

std::vector<jl::x86::VirtualRegister> jl::x86::Pop::uses() const
{
    return {};
}

void jl::x86::Pop::replace(jl::x86::VirtualRegister r, jl::x86::Operand operand)
{
}

void jl::x86::Pop::accept(jl::x86::InstructionVisitor& visitor)
{
    visitor.visit(*this);
}

//============================JUMP=============================

std::string jl::x86::Jump::to_str() const
{
    return "jmp " + target->m_name;
}

std::vector<jl::x86::VirtualRegister> jl::x86::Jump::defs() const
{
    return {};
}

std::vector<jl::x86::VirtualRegister> jl::x86::Jump::uses() const
{
    return {};
}

void jl::x86::Jump::replace(jl::x86::VirtualRegister reg, jl::x86::Operand operand) { }

void jl::x86::Jump::accept(jl::x86::InstructionVisitor& visitor)
{
    visitor.visit(*this);
}

//============================JUMP-EQUAL=============================

std::string jl::x86::JumpEqual::to_str() const
{
    return "je " + target->m_name;
}

std::vector<jl::x86::VirtualRegister> jl::x86::JumpEqual::defs() const
{
    return {};
}

std::vector<jl::x86::VirtualRegister> jl::x86::JumpEqual::uses() const
{
    return {};
}

void jl::x86::JumpEqual::replace(jl::x86::VirtualRegister reg, jl::x86::Operand operand) { }

void jl::x86::JumpEqual::accept(jl::x86::InstructionVisitor& visitor)
{
    visitor.visit(*this);
}

//============================CMP=============================

std::string jl::x86::Cmp::to_str() const
{
    OperandPrinter printer;
    return "cmp "
        + a.to_str()
        + ", " + b.to_str();
}

std::vector<jl::x86::VirtualRegister> jl::x86::Cmp::defs() const
{
    return {};
}

std::vector<jl::x86::VirtualRegister> jl::x86::Cmp::uses() const
{
    return { a, b };
}

void jl::x86::Cmp::replace(jl::x86::VirtualRegister r, jl::x86::Operand operand)
{
}

void jl::x86::Cmp::accept(jl::x86::InstructionVisitor& visitor)
{
    visitor.visit(*this);
}

//============================LEA=============================

jl::x86::Lea::Lea(
    jl::x86::VirtualRegister source,
    VirtualRegister dest)
    : source(source)
    , dest(dest)
{
}

std::string jl::x86::Lea::to_str() const
{
    OperandPrinter printer;
    return "lea "
        + dest.to_str()
        + ", " + source.to_str();
}

std::vector<jl::x86::VirtualRegister> jl::x86::Lea::defs() const
{
    return { dest };
}

std::vector<jl::x86::VirtualRegister> jl::x86::Lea::uses() const
{
    return { source };
}

void jl::x86::Lea::replace(jl::x86::VirtualRegister r, jl::x86::Operand operand)
{
}

void jl::x86::Lea::accept(jl::x86::InstructionVisitor& visitor)
{
    visitor.visit(*this);
}
