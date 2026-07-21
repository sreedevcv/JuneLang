#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <variant>

namespace jl {
namespace x86 {
    struct PhysicalRegister {
        enum Type {
            rax,
            rbx,
            rcx,
            rdx,
            rsi,
            rdi,
            rbp,
            rsp,
            r8,
            r9,
            r10,
            r11,
            r12,
            r13,
            r14,
            r15
        } reg;

        inline std::string to_string() const
        {
            switch (reg) {
            case rax:
                return "rax";
            case rbx:
                return "rbx";
            case rcx:
                return "rcx";
            case rdx:
                return "rdx";
            case rsi:
                return "rsi";
            case rdi:
                return "rdi";
            case rbp:
                return "rbp";
            case rsp:
                return "rsp";
            case r8:
                return "r8";
            case r9:
                return "r9";
            case r10:
                return "r10";
            case r11:
                return "r11";
            case r12:
                return "r12";
            case r13:
                return "r13";
            case r14:
                return "r14";
            case r15:
                return "r15";
                break;
            }
        }
    };

    struct VirtualRegister {
        uint32_t id;
        std::optional<PhysicalRegister> hint;

        inline VirtualRegister()
            : id(UINT32_MAX)
            , hint(std::nullopt)
        {
        }

        inline VirtualRegister(uint32_t idx, std::optional<PhysicalRegister> hint = std::nullopt)
            : id(idx)
            , hint(hint)
        {
        }

        inline std::string to_string() const
        {
            std::string s = "t" + std::to_string(id);
            if (hint) {
                s += "(" + hint->to_string() + ")";
            }
            return s;
        }
    };

    using Register = std::variant<PhysicalRegister, VirtualRegister>;

    struct RegisterPrinter {
        std::string operator()(const PhysicalRegister& reg) const
        {
            return reg.to_string();
        }

        std::string operator()(const VirtualRegister& reg)
        {
            return reg.to_string();
        }
    };

}
}