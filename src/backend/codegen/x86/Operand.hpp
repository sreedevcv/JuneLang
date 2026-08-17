#pragma once

#include "codegen/x86/Register.hpp"

#include <string>

namespace jl {
namespace x86 {

    struct MemoryOperand {
        // [base + scale * index + displacement]
        VirtualRegister base;
        std::optional<VirtualRegister> index;
        uint32_t scale = 1;
        int32_t displacement = 0;
        std::optional<SizeDirective> size;

        inline std::string to_str() const
        {
            std::string addr = base.to_str();
            auto size_dir = (size ? jl::x86::to_str(*size) : "");
            if (index) {
                addr += std::to_string(scale) + " * " + index->to_str();
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
            return mem.to_str();
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
            return {}; 
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
}
}
