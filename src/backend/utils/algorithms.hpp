#pragma once

#include <unordered_map>
#include <utility>

#include "BasicBlock.hpp"
#include "Function.hpp"

namespace jl {
namespace algorithms {

    std::pair<BasicBlock*, BasicBlock*> get_sucessors(BasicBlock* block);

    std::unordered_map<BasicBlock*, std::vector<BasicBlock*>> get_predecessors(Function* function);

    std::unordered_map<jl::BasicBlock*, uint32_t> RPO(BasicBlock* entry_block);

    std::unordered_map<BasicBlock*, BasicBlock*> dominance_tree(Function* function);

    using DominanceFrontier = std::unordered_map<jl::BasicBlock*, std::vector<jl::BasicBlock*>>;
    DominanceFrontier dominance_frontier(jl::Function* function, std::vector<jl::BasicBlock*>& rpo);

}
}
