#pragma once

#include "codegen/x86/MachineBlock.hpp"
#include "codegen/x86/Register.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace jl {
namespace x86 {
    struct Instruction {
        uint32_t m_id = 0;

        virtual ~Instruction() = default;

        virtual std::string to_string() const = 0;

        virtual std::vector<VirtualRegister> defs() const = 0;

        virtual std::vector<VirtualRegister> uses() const = 0;
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
        VirtualRegister base;
        std::optional<VirtualRegister> index;
        uint32_t scale = 1;
        int32_t displacement = 0;
        std::optional<SizeDirective> size;

        inline std::string to_string() const
        {
            std::string addr = base.to_string();
            auto size_dir = (size ? jl::x86::to_string(*size) + " PTR " : "");
            if (index) {
                addr += std::to_string(scale) + " * " + index->to_string();
            }
            if (displacement != 0) {
                addr += std::to_string(displacement);
            }
            return size_dir + "[" + addr + "]";
        }
    };

    using Operand = std::variant<VirtualRegister, MemoryOperand, int64_t>;

    struct GetDefinedRegs {
        std::vector<VirtualRegister> operator()(const VirtualRegister& vreg) const
        {
            return { vreg };
        }
        std::vector<VirtualRegister> operator()(const MemoryOperand&) const
        {
            return {}; // Writing to memory does NOT define a register
        }
        std::vector<VirtualRegister> operator()(const int64_t&) const
        {
            return {};
        }
    };

    struct GetUsedRegs {
        std::vector<VirtualRegister> operator()(const MemoryOperand& mem) const
        {
            std::vector<VirtualRegister> regs = { mem.base };
            if (mem.index) {
                regs.push_back(*mem.index);
            }
            return regs;
        }
        std::vector<VirtualRegister> operator()(const VirtualRegister& vreg) const
        {
            return { vreg };
        }
        std::vector<VirtualRegister> operator()(const int64_t&) const
        {
            return {};
        }
    };

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

        std::vector<VirtualRegister> defs() const override
        {
            return std::visit(GetDefinedRegs {}, dest);
        }

        std::vector<VirtualRegister> uses() const override
        {
            auto src_uses = std::visit(GetUsedRegs {}, source);
            if (std::holds_alternative<MemoryOperand>(dest)) {
                const auto dest_uses = std::visit(GetUsedRegs {}, dest);
                src_uses.insert(src_uses.end(), dest_uses.begin(), dest_uses.end());
            }

            return src_uses;
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

        std::vector<VirtualRegister> defs() const override
        {
            return std::visit(GetDefinedRegs {}, dest);
        }

        std::vector<VirtualRegister> uses() const override
        {
            auto src_uses = std::visit(GetUsedRegs {}, source);
            if (std::holds_alternative<MemoryOperand>(dest)) {
                const auto dest_uses = std::visit(GetUsedRegs {}, dest);
                src_uses.insert(src_uses.end(), dest_uses.begin(), dest_uses.end());
            }

            return src_uses;
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

        std::vector<VirtualRegister> defs() const override
        {
            return std::visit(GetDefinedRegs {}, dest);
        }

        std::vector<VirtualRegister> uses() const override
        {
            auto src_uses = std::visit(GetUsedRegs {}, source);
            if (std::holds_alternative<MemoryOperand>(dest)) {
                const auto dest_uses = std::visit(GetUsedRegs {}, dest);
                src_uses.insert(src_uses.end(), dest_uses.begin(), dest_uses.end());
            }

            return src_uses;
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

        std::vector<VirtualRegister> defs() const override
        {
            return { reg };
        }

        std::vector<VirtualRegister> uses() const override
        {
            return {};
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

        std::vector<VirtualRegister> defs() const override
        {
            return { reg };
        }

        std::vector<VirtualRegister> uses() const override
        {
            return {};
        }
    };

    struct Return : public Instruction {
        ~Return() = default;

        inline std::string to_string() const override
        {
            return "ret";
        }

        std::vector<VirtualRegister> defs() const override
        {
            return {};
        }

        std::vector<VirtualRegister> uses() const override
        {
            return {};
        }
    };

    struct Push : public Instruction {
        Operand value;

        ~Push() = default;

        inline std::string to_string() const override
        {
            return "push " + std::visit(OperandPrinter {}, value);
        }

        std::vector<VirtualRegister> defs() const override
        {
            return {};
        }

        std::vector<VirtualRegister> uses() const override
        {
            return std::visit(GetUsedRegs {}, value);
        }
    };

    struct Pop : public Instruction {
        Operand value;

        ~Pop() = default;

        inline std::string to_string() const override
        {
            return "pop " + std::visit(OperandPrinter {}, value);
        }

        std::vector<VirtualRegister> defs() const override
        {
            return std::visit(GetDefinedRegs {}, value);
        }

        std::vector<VirtualRegister> uses() const override
        {
            if (std::holds_alternative<MemoryOperand>(value)) {
                return std::visit(GetUsedRegs {}, value);
            }
            return {};
        }
    };

    struct Jump : public Instruction {
        MachineBlock* target;

        ~Jump() = default;

        inline std::string to_string() const override
        {
            return "jmp " + target->m_name;
        }

        std::vector<VirtualRegister> defs() const override
        {
            return {};
        }

        std::vector<VirtualRegister> uses() const override
        {
            return {};
        }
    };

    struct JumpEqual : public Jump {
        ~JumpEqual() = default;

        inline std::string to_string() const override
        {
            return "je " + target->m_name;
        }

        std::vector<VirtualRegister> defs() const override
        {
            return {};
        }

        std::vector<VirtualRegister> uses() const override
        {
            return {};
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

        std::vector<VirtualRegister> defs() const override
        {
            return {};
        }

        std::vector<VirtualRegister> uses() const override
        {
            auto a_uses = std::visit(GetUsedRegs {}, a);
            auto b_uses = std::visit(GetUsedRegs {}, b);
            a_uses.insert(a_uses.begin(), b_uses.begin(), b_uses.end());
            return a_uses;
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

        std::vector<VirtualRegister> defs() const override
        {
            return { dest };
        }

        std::vector<VirtualRegister> uses() const override
        {
            return std::visit(GetUsedRegs {}, Operand(source));
        }
    };
}
}
