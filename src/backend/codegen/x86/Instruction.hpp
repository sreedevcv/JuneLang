#pragma once

#include "codegen/x86/MachineBlock.hpp"
#include "codegen/x86/Register.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <variant>

namespace jl {
namespace x86 {
    struct Instruction {
        uint32_t m_id = 0;
        virtual std::string to_string() const = 0;
        virtual ~Instruction() = default;
    };

    enum class SizeDirective {
        BYTE,
        WORD,
        DWORD,
        QWORD,
    };

    inline std::string to_string(const SizeDirective& dir)
    {
        switch (dir) {
        case SizeDirective::BYTE:
            return "BYTE";
        case SizeDirective::WORD:
            return "WORD";
        case SizeDirective::DWORD:
            return "DWORD";
        case SizeDirective::QWORD:
            return "QWORD";
        }
    }

    struct MemoryOperand {
        // [base + scale * index + displacement]
        Register base;
        std::optional<Register> index;
        uint32_t scale = 1;
        int32_t displacement = 0;
        std::optional<SizeDirective> size;

        inline std::string to_string() const
        {
            std::string addr = std::visit(RegisterPrinter {}, base);
            auto size_dir = (size ? jl::x86::to_string(*size) + " PTR " : "");
            if (index) {
                addr += std::to_string(scale) + " * " + std::visit(RegisterPrinter {}, *index);
            }
            if (displacement != 0) {
                addr += std::to_string(displacement);
            }
            return size_dir + "[" + addr + "]";
        }
    };

    using Operand = std::variant<Register, MemoryOperand, int64_t>;

    struct OperandPrinter {

        std::string operator()(const Register& reg) const
        {
            return std::visit(RegisterPrinter {}, reg);
        }

        std::string operator()(const MemoryOperand& mem) const
        {
            return mem.to_string();
        }

        std::string operator()(const int64_t& imm) const
        {
            return std::to_string(imm);
        }
    };

    struct Mov : public Instruction {
        Operand source;
        Operand dest;
        bool is_float;

        ~Mov() = default;

        inline std::string to_string() const override
        {
            OperandPrinter printer;
            return "mov "
                + std::visit(printer, dest)
                + ", " + std::visit(printer, source);
        }
    };

    struct Add : public Instruction {
        Operand source;
        Operand dest;
        bool is_float;

        ~Add() = default;

        inline std::string to_string() const override
        {
            OperandPrinter printer;
            return "add "
                + std::visit(printer, dest)
                + ", " + std::visit(printer, source);
        }
    };

    struct Sub : public Instruction {
        Operand source;
        Operand dest;
        bool is_float;

        ~Sub() = default;

        inline std::string to_string() const override
        {
            OperandPrinter printer;
            return "sub "
                + std::visit(printer, dest)
                + ", " + std::visit(printer, source);
        }
    };

    struct Less : public Instruction {
        VirtualRegister reg;
        bool is_float;

        ~Less() = default;

        inline std::string to_string() const override
        {
            return "setl " + reg.to_string();
        }
    };

    struct Equals : public Instruction {
        VirtualRegister reg;
        bool is_float;

        ~Equals() = default;

        inline std::string to_string() const override
        {
            return "sete " + reg.to_string();
        }
    };

    struct Return : public Instruction {
        ~Return() = default;

        inline std::string to_string() const override
        {
            return "ret";
        }
    };

    struct Push : public Instruction {
        Operand value;

        ~Push() = default;

        inline std::string to_string() const override
        {
            return "push " + std::visit(OperandPrinter {}, value);
        }
    };

    struct Pop : public Instruction {
        Operand value;

        ~Pop() = default;

        inline std::string to_string() const override
        {
            return "pop " + std::visit(OperandPrinter {}, value);
        }
    };

    struct Jump : public Instruction {
        MachineBlock* target;

        ~Jump() = default;

        inline std::string to_string() const override
        {
            return "jmp " + target->m_name;
        }
    };

    struct JumpEqual : public Jump {
        ~JumpEqual() = default;

        inline std::string to_string() const override
        {
            return "je " + target->m_name;
        }
    };

    struct Cmp : public Instruction {
        Operand a;
        Operand b;
        bool is_float;

        ~Cmp() = default;

        inline std::string to_string() const override
        {
            OperandPrinter printer;
            return "cmp "
                + std::visit(printer, a)
                + ", " + std::visit(printer, b);
        }
    };

    struct Lea : public Instruction {
        MemoryOperand source;
        VirtualRegister dest;
        bool is_float;

        Lea(
            MemoryOperand source,
            VirtualRegister dest)
            : source(source)
            , dest(dest)
        {
        }

        ~Lea() = default;

        inline std::string to_string() const override
        {
            OperandPrinter printer;
            return "lea "
                + dest.to_string()
                + ", " + source.to_string();
        }
    };
}
}