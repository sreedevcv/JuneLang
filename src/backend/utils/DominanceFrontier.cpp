#include "algorithms.hpp"
#include <print>

jl::algorithms::DominanceFrontier jl::algorithms::dominance_frontier(jl::Function* function, std::vector<jl::BasicBlock*>& rpo)
{
    DominanceFrontier dom_frontier;
    auto dom_tree = jl::algorithms::dominance_tree(function);

    auto predecessors = jl::algorithms::get_predecessors(function);

    for (auto block : rpo) {
        if (predecessors[block].size() >= 2) {
            // Check if a block's predecesor is not dominated by that blocks dominator
            for (auto pred : predecessors[block]) {
                auto runner = pred;

                while (runner != dom_tree[block] && runner != block) {
                    dom_frontier[runner].push_back(block);
                    runner = dom_tree[runner];
                }

                /*
                while (runner != dom_tree[block]) {
        dom_frontier[runner].push_back(block);

        // If runner reaches the entry block (which points to itself),
        // or if it somehow matches the block we are looking at to prevent self-inclusion
        if (runner == dom_tree[runner]) {
            break;
        }

        runner = dom_tree[runner];
    }
                */
            }
        }
    }

    std::println("-------------------------DT-START---------------------------");
    for (const auto& [block, dom] : dom_tree) {
        std::println("{} -> {}: ", block->get_name(), dom->get_name());
    }
    std::println("-------------------------DT-END---------------------------");

    std::println("-------------------------DF-START---------------------------");
    for (const auto& [block, frontier] : dom_frontier) {
        std::print("{}: ", block->get_name());
        for (const auto b : frontier) {
            std::print("{}, ", b->get_name());
        }
        std::println();
    }
    std::println("-------------------------DF-END---------------------------");

    return dom_frontier;
}
