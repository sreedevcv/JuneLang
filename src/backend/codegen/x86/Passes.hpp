#pragma once

#include "codegen/x86/MachineFunction.hpp"

namespace jl {
namespace x86 {
    namespace pass {

        struct LiveInterval {
            uint32_t start = UINT32_MAX;
            uint32_t end = 0;
        };

        using LiveIntervalMap = std::unordered_map<jl::x86::VirtualRegister, LiveInterval, jl::x86::VirtualRegisterHasher>;

        LiveIntervalMap liveness_analysis(MachineFunction* function);

        void linear_scan_reg_allocation(MachineFunction* function, const LiveIntervalMap& intervals);
    }
}
}
