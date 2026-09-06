#pragma once

#include "codegen/x86/MachineFunction.hpp"
#include "codegen/x86/Range.hpp"
#include <unordered_set>

namespace jl {
namespace x86 {

    class LivenessAnalysis {
    public:
        LivenessAnalysis(MachineFunction* function);

        LiveIntervalMap calculate_live_intervals();

    private:
        void number_instructions();

        void liveness_analysis();

        using reg_set = std::unordered_set<jl::x86::VirtualRegister, jl::x86::VirtualRegisterHasher>;

        jl::x86::MachineFunction* function;
        std::vector<jl::x86::MachineBlock*> rpo;
        std::unordered_map<jl::x86::MachineBlock*, reg_set> gen; /* uses */
        std::unordered_map<jl::x86::MachineBlock*, reg_set> kill; /* defs */
        std::unordered_map<jl::x86::MachineBlock*, reg_set> live_in;
        std::unordered_map<jl::x86::MachineBlock*, reg_set> live_out;
        std::unordered_map<jl::x86::MachineBlock*, std::vector<jl::x86::MachineBlock*>> successors;
    };
}
}
