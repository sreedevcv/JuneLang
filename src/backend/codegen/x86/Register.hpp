#pragma once

#include "Utils.hpp"
#include <cstdint>
#include <functional>
#include <optional>
#include <print>
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
            r15,
            REG_SENTINEL
        } reg;

        bool is_byte = false;

        inline std::string to_str() const
        {
            switch (reg) {
            case rax:
                return is_byte ? "al" : "rax";
            case rbx:
                return is_byte ? "bl" : "rbx";
            case rcx:
                return is_byte ? "cl" : "rcx";
            case rdx:
                return is_byte ? "dl" : "rdx";
            case rsi:
                return is_byte ? "sil" : "rsi";
            case rdi:
                return is_byte ? "dil" : "rdi";
            case rbp:
                return is_byte ? "bpl" : "rbp";
            case rsp:
                return is_byte ? "spl" : "rsp";
            case r8:
                return is_byte ? "r8b" : "r8";
            case r9:
                return is_byte ? "r9b" : "r9";
            case r10:
                return is_byte ? "r10b" : "r10";
            case r11:
                return is_byte ? "r11b" : "r11";
            case r12:
                return is_byte ? "r12b" : "r12";
            case r13:
                return is_byte ? "r13b" : "r13";
            case r14:
                return is_byte ? "r14b" : "r14";
            case r15:
                return is_byte ? "r15b" : "r15";
            case REG_SENTINEL:
                return is_byte ? "REGISTER_MAX" : "REGISTER_MAX";
            }

            std::println("invalid register: {}", static_cast<int>(reg));
            unimplemented();
            return "";
        }
    };

    enum class SizeDirective {
        BYTE,
        WORD,
        DWORD,
        QWORD,
        NONE,
    };

    inline std::string to_str(const SizeDirective& dir)
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
        case SizeDirective::NONE:
            return "NONE";
        }
    }

    struct VirtualRegister {
        uint32_t id;
        int32_t allocation;
        SizeDirective size;

        static inline bool debug_print = true;

        inline VirtualRegister()
            : id(UINT32_MAX)
            , allocation(-1)
            , size(SizeDirective::NONE)
        {
        }
        
        inline VirtualRegister(uint32_t vid)
            : id(vid)
            , allocation(-1)
            , size(SizeDirective::NONE)
        {
        }

        inline std::string to_str() const
        {
            std::string s = "t" + std::to_string(id);

            if (size != SizeDirective::NONE) {
                s += "(" + jl::x86::to_str(size) + ")";
            }
            
            return s;
        }

        inline bool operator==(const VirtualRegister& reg) const
        {
            return id == reg.id;
        }
    };

    struct VirtualRegisterHasher {
        std::size_t operator()(const VirtualRegister& reg) const
        {
            auto hash1 = std::hash<uint32_t> {}(reg.id);
            return hash1;
        }
    };

    using Register
        = std::variant<PhysicalRegister, VirtualRegister>;

    struct RegisterPrinter {
        std::string operator()(const PhysicalRegister& reg) const
        {
            return reg.to_str();
        }

        std::string operator()(const VirtualRegister& reg)
        {
            return reg.to_str();
        }
    };

}
}
