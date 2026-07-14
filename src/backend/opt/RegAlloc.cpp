
#include "Function.hpp"
#include "ir/IR.hpp"
#include "ir/Phi.hpp"
#include <cstdint>
#include <unordered_map>

struct RegAllocState {
};

struct LiveInterval {
    jl::ir::IR* start = nullptr;
    jl::ir::IR* end = nullptr;

    void insert(jl::ir::IR* ir)
    {
        if (start == nullptr) {
            start = ir;
        }

        end = ir;
    }
};

std::unordered_map<uint32_t, LiveInterval> find_liveness(jl::Function* function)
{
    std::unordered_map<uint32_t, LiveInterval> liveness;

    for (auto& block : function->blocks()) {
        for (auto phi : block->phis) {
            liveness[phi->m_dest.id()].insert(phi);
        }

        for (auto ir = block->head; ir != nullptr; ir = ir->next) {
        }
    }
}

void regalloc(jl::Function* function)
{
    std::unordered_map<uint32_t, LiveInterval> liveness;
}