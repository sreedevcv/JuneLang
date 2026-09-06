#pragma once

#include "codegen/x86/MachineFunction.hpp"
#include "codegen/x86/Range.hpp"
#include "codegen/x86/Register.hpp"
#include <unordered_set>

namespace jl {
namespace x86 {
    namespace pass {
        using reg_set = std::unordered_set<jl::x86::VirtualRegister, jl::x86::VirtualRegisterHasher>;

        LiveIntervalMap liveness_analysis(MachineFunction* function);

        AllocationMap linear_scan_reg_allocation(jl::x86::MachineFunction* function, const LiveIntervalMap& intervals, uint8_t gpr_count, uint8_t float_count);

        void assign_register(MachineFunction* function, AllocationMap allocations);

        struct AssemblyProgram {
            std::string text_section;
            std::string data_section;
        };

        void to_nasm_assembly(AssemblyProgram& program, MachineFunction* function);
    }

}
}
