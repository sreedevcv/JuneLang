#pragma once

#include "Utils.hpp"
#include <cstdint>
#include <functional>
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
            xmm0,
            xmm1,
            xmm2,
            xmm3,
            xmm4,
            xmm5,
            xmm6,
            xmm7,
            xmm8,
            xmm9,
            xmm10,
            xmm11,
            xmm12,
            xmm13,
            xmm14,
            xmm15,
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
            case xmm0:
                return "xmm0";
            case xmm1:
                return "xmm1";
            case xmm2:
                return "xmm2";
            case xmm3:
                return "xmm3";
            case xmm4:
                return "xmm4";
            case xmm5:
                return "xmm5";
            case xmm6:
                return "xmm6";
            case xmm7:
                return "xmm7";
            case xmm8:
                return "xmm8";
            case xmm9:
                return "xmm9";
            case xmm10:
                return "xmm10";
            case xmm11:
                return "xmm11";
            case xmm12:
                return "xmm12";
            case xmm13:
                return "xmm13";
            case xmm14:
                return "xmm14";
            case xmm15:
                return "xmm15";
            case REG_SENTINEL:
                return "REGISTER_MAX";
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
        bool is_float = false;
        SizeDirective size;

        static inline bool debug_print = true;

        inline VirtualRegister()
            : id(UINT32_MAX)
            , is_float(false)
            , size(SizeDirective::NONE)
        {
        }

        inline VirtualRegister(uint32_t vid, bool is_float = false)
            : id(vid)
            , is_float(is_float)
            , size(SizeDirective::NONE)
        {
        }

        inline std::string to_str() const
        {
            std::string s = "t" + std::to_string(id);

            if (size != SizeDirective::NONE) {
                s += "(" + jl::x86::to_str(size) + ")";
            }
            if (is_float) {
                s += "(flt)";
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

    const PhysicalRegister::Type input_gpr_registers[] = {
        PhysicalRegister::rdi,
        PhysicalRegister::rsi,
        PhysicalRegister::rdx,
        PhysicalRegister::rcx,
        PhysicalRegister::r8,
        PhysicalRegister::r9
    };

    const PhysicalRegister::Type input_float_registers[] = {
        PhysicalRegister::xmm0,
        PhysicalRegister::xmm1,
        PhysicalRegister::xmm2,
        PhysicalRegister::xmm3,
        PhysicalRegister::xmm4,
        PhysicalRegister::xmm5,
        PhysicalRegister::xmm6,
        PhysicalRegister::xmm7,
    };
}
}
