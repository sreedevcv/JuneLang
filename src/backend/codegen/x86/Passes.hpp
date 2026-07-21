#pragma once

#include "codegen/x86/MachineFunction.hpp"

namespace jl {
namespace x86 {
    namespace pass {
        void number_instructions(MachineFunction* function);
    }
}
}