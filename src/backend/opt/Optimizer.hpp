#pragma once

#include "Function.hpp"

namespace jl {
namespace opt {
    void mem2reg(Function* function);
    void sccp(Function* function);
    void remove_phi_nodes(jl::Function* function);
}
}
