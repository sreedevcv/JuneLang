#pragma once

#include "codegen/x86/Register.hpp"

#include <cstdint>
#include <string>

namespace jl {
namespace x86 {
    struct Allocation {
        enum Type {
            GPR,
            FLOAT,
            SLOT,
        };

        Type type;
        uint32_t value;

        inline std::string to_str() const
        {
            std::string s;
            switch (type) {
            case GPR:
                s = "gpr";
                break;
            case FLOAT:
                s = "flt";
                break;
            case SLOT:
                s = "stk";
                break;
            }
            s += std::to_string(value);
            return s;
        }
    };

    struct Range {
        int32_t start = -1;
        int32_t end = -1;
        VirtualRegister vreg;

        inline void add_range(int32_t new_start, int32_t new_end)
        {
            start = start == -1 ? new_start : std::min(start, new_start);
            end = end == -1 ? new_end : std::max(end, new_end);
        }

        inline void set_start(int32_t new_start)
        {
            // assert(end == -1 || new_start <= end);
            start = new_start;
        }

        inline bool contains(int32_t point) const
        {
            return point >= start && point <= end;
        }

        inline bool operator==(const Range& other) const
        {
            return start == other.start && end == other.end;
        }
    };

    using LiveIntervalMap = std::unordered_map<jl::x86::VirtualRegister, Range, jl::x86::VirtualRegisterHasher>;
    using AllocationMap = std::unordered_map<jl::x86::VirtualRegister, Allocation, jl::x86::VirtualRegisterHasher>;

    struct RangeCompare {
        bool operator()(const Range& a, const Range& b) const
        {
            if (a.end != b.end) {
                return a.end < b.end;
            } else if (a.start != b.start) {
                return a.start < b.start;
            } 
            return a.vreg.id < b.vreg.id;
        }
    };

    struct RangeHasher {
        std::size_t operator()(const Range& range) const
        {
            return VirtualRegisterHasher {}(range.vreg);
        }
    };
}
}
