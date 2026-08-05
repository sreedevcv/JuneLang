#pragma once

#include "codegen/x86/Register.hpp"

#include <string>

namespace jl {
namespace x86 {
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
            auto size_dir = (size ? jl::x86::to_string(*size) : "");
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

    struct OperandReplacer {
        const VirtualRegister& target;
        const Operand& operand;

        OperandReplacer(const VirtualRegister& target, Operand& replacement)
            : target(target)
            , operand(operand)
        {
        }

        // void operator()(MemoryOperand& mem)
        //{
        // if (mem.base == replacement) {
        // mem.base = replacement;
        //} else if (mem.index && *mem.index == replacement) {
        // mem.index = replacement;
        //}
        //}
        //
        // void operator()(VirtualRegister& vreg)
        //{
        // if (vreg == replacement) {
        // vreg = replacement;
        //}
        //}
        //
        void operator()(const int64_t&) const { }
    };
}
}
