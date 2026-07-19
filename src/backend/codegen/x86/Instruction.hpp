#pragma once

#include "codegen/x86/Register.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <variant>

namespace jl {
namespace x86 {
    struct Instruction {
        uint32_t m_line = 0;
        virtual std::string to_string() const = 0;
        virtual ~Instruction() = default;
    };

    struct MemoryOperand {
        // [base + scale * index + displacement]
        Register base;
        std::optional<Register> index;
        uint32_t scale = 1;
        int32_t displacement = 0;

        inline std::string to_string() const
        {
            std::string addr = std::visit(RegisterPrinter {}, base);
            if (index) {
                addr += std::to_string(scale) + " * " + std::visit(RegisterPrinter {}, *index);
            }
            if (displacement != 0) {
                addr += std::to_string(displacement);
            }
            return "[" + addr + "]";
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

    struct Mov : public Instruction {
        Operand source;
        Operand dest;
        std::optional<SizeDirective> size;
        bool is_float;

        ~Mov() = default;

        inline std::string to_string() const override
        {
            OperandPrinter printer;
            return "mov "
                + (size ? jl::x86::to_string(*size) + " PTR " : "")
                + std::visit(printer, dest)
                + ", " + std::visit(printer, source);
        }
    };

    struct Add : public Instruction {
        Operand source;
        Operand dest;
        std::optional<SizeDirective> size;
        bool is_float;

        ~Add() = default;

        inline std::string to_string() const override
        {
            OperandPrinter printer;
            return "add "
                + (size ? jl::x86::to_string(*size) + " PTR " : "")
                + std::visit(printer, dest)
                + ", " + std::visit(printer, source);
        }
    };

    struct Sub : public Instruction {
        Operand source;
        Operand dest;
        std::optional<SizeDirective> size;
        bool is_float;

        ~Sub() = default;

        inline std::string to_string() const override
        {
            OperandPrinter printer;
            return "sub "
                + (size ? jl::x86::to_string(*size) + " PTR " : "")
                + std::visit(printer, dest)
                + ", " + std::visit(printer, source);
        }
    };

    struct Less : public Instruction {
        Operand source;
        Operand dest;
        std::optional<SizeDirective> size;
        bool is_float;

        ~Less() = default;

        inline std::string to_string() const override
        {
            OperandPrinter printer;
            return "setl "
                + (size ? jl::x86::to_string(*size) + " PTR " : "")
                + std::visit(printer, dest)
                + ", " + std::visit(printer, source);
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
        std::string label;

        ~Jump() = default;

        inline std::string to_string() const override
        {
            return "jmp " + label;
        }
    };

    struct JumpEqual : public Instruction {
        std::string label;

        ~JumpEqual() = default;

        inline std::string to_string() const override
        {
            return "je " + label;
        }
    };

    struct Cmp : public Instruction {
        Operand source;
        Operand dest;
        std::optional<SizeDirective> size;
        bool is_float;

        ~Cmp() = default;

        inline std::string to_string() const override
        {
            OperandPrinter printer;
            return "cmp "
                + (size ? jl::x86::to_string(*size) + " PTR " : "")
                + std::visit(printer, dest)
                + ", " + std::visit(printer, source);
        }
    };
}
}