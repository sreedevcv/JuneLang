#pragma once

#include "RegisterAllocator.hpp"
#include "codegen/x86/MachineFunction.hpp"
#include <cstdint>
#include <variant>

namespace jl {
namespace x86 {
    namespace pass {
        using LiveIntervalMap = std::unordered_map<jl::x86::VirtualRegister, Range, jl::x86::VirtualRegisterHasher>;
        using reg_set = std::unordered_set<jl::x86::VirtualRegister, jl::x86::VirtualRegisterHasher>;
        using AllocationMap = std::unordered_map<jl::x86::VirtualRegister, jl::Allocation, jl::x86::VirtualRegisterHasher>;

        LiveIntervalMap liveness_analysis(MachineFunction* function);

        AllocationMap linear_scan_reg_allocation(jl::x86::MachineFunction* function, const jl::x86::pass::LiveIntervalMap& intervals);

        void assign_register(MachineFunction* function, AllocationMap allocations);
    }
}
}
