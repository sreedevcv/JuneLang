#include "algorithms.hpp"

#include <cstdint>
#include <print>
#include <unordered_map>
#include <vector>

#include "BasicBlock.hpp"

jl::BasicBlock* find_first_processed_predecessor(jl::BasicBlock* block,
    std::unordered_map<jl::BasicBlock*, jl::BasicBlock*>& doms,
    std::unordered_map<jl::BasicBlock*, std::vector<jl::BasicBlock*>>& preds)
{
    for (auto p : preds[block]) {
        if (doms[p] != nullptr) {
            return p;
        }
    }

    return nullptr;
}

jl::BasicBlock* intersect(jl::BasicBlock* b1,
    jl::BasicBlock* b2,
    std::unordered_map<jl::BasicBlock*, uint32_t>& rpo,
    std::unordered_map<jl::BasicBlock*, jl::BasicBlock*>& doms)
{
    auto ptr1 = b1;
    auto ptr2 = b2;

    while (ptr1 != ptr2) {
        while (rpo[ptr1] > rpo[ptr2])
            ptr1 = doms[ptr1];
        while (rpo[ptr2] > rpo[ptr1])
            ptr2 = doms[ptr2];
    }

    return ptr1;
}

std::unordered_map<jl::BasicBlock*, jl::BasicBlock*> jl::algorithms::dominance_tree(Function* function)
{
    std::unordered_map<BasicBlock*, BasicBlock*> dom_tree;

    for (auto& block : function->blocks()) {
        dom_tree[block.get()] = nullptr;
    }

    auto entry_block = function->entry_block();
    dom_tree[entry_block] = entry_block;

    auto rpo_map = RPO(entry_block);
    auto predecessors = get_predecessors(function);
    bool changed = true;

    std::vector<BasicBlock*> rpo(rpo_map.size(), nullptr);
    for (auto& [block, idx] : rpo_map) {
        rpo[idx] = block;
    }

    while (changed) {
        changed = false;

        for (int i = 1; i < rpo.size(); i++) {
            auto block = rpo[i];
            auto new_idom = find_first_processed_predecessor(block, dom_tree, predecessors);

            for (auto pred : predecessors[block]) {
                if (pred != new_idom && dom_tree[pred] != nullptr) {
                    new_idom = intersect(pred, new_idom, rpo_map, dom_tree);
                }
            }

            if (dom_tree[block] != new_idom) {
                dom_tree[block] = new_idom;
                changed = true;
            }
        }
    }

    return dom_tree;
}
