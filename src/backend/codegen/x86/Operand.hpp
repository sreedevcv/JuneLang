#pragma once

#include "codegen/x86/Register.hpp"

#include <cstdint>
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

    struct MemoryLabel {
        std::string label;
        SizeDirective size;
    };

    using MachineAlloc = std::variant<PhysicalRegister, MemoryOperand, MemoryLabel, int64_t>;

    // Checks if size is 1, 2, 4 or 8 and returns is as PTR otherwise we need to do memcpy to move
    inline std::optional<SizeDirective> is_simple_move(uint32_t size)
    {
        switch (size) {
        case 0:
            unimplemented();
        case 1:
            return SizeDirective::BYTE;
        case 2:
            return SizeDirective::WORD;
        case 4:
            return SizeDirective::DWORD;
        case 8:
            return SizeDirective::QWORD;
        default:
            return std::nullopt;
        }
    }

    struct StaticData {
        enum size {
            dw,
            dq
        };

        std::string label;
        size size;
        double value;

        std::string to_str() const
        {
            std::string s = label;
            switch (size) {
            case dw:
                s += " dw ";
                break;
            case dq:
                s += " dq ";
                break;
            }

            return s + std::to_string(value);
        }
    };
}
}
