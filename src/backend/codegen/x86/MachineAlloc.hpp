#pragma once

#include "codegen/x86/Operand.hpp"

namespace jl {
namespace x86 {
    using MachineAlloc = std::variant<PhysicalRegister, MemoryOperand, MemoryLabel, int64_t>;

}

}
