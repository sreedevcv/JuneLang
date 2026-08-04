#pragma once

#include "codegen/x86/MachineFunction.hpp"
#include <cstdint>
#include <variant>

namespace jl {
namespace x86 {
    namespace pass {

        struct LiveInterval {
            uint32_t start = UINT32_MAX;
            uint32_t end = 0;

            inline bool contains(uint32_t point) const
            {
                return point >= start && point <= end;
            }
        };

        using LiveIntervalMap = std::unordered_map<jl::x86::VirtualRegister, LiveInterval, jl::x86::VirtualRegisterHasher>;

        LiveIntervalMap liveness_analysis(MachineFunction* function);

        using StackSlot = uint32_t;
        using Allocation = std::variant<PhysicalRegister, StackSlot>;

        struct AllocationRange {
            Allocation allocation;
            LiveInterval interval;
        };

        using RegisterAllocationMap = std::unordered_map<VirtualRegister, std::vector<AllocationRange>, VirtualRegisterHasher>;

        RegisterAllocationMap linear_scan_reg_allocation(MachineFunction* function, const LiveIntervalMap& intervals);
    }
}
}
