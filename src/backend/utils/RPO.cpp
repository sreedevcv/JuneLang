#include "algorithms.hpp"

#include <stack>
#include <unordered_map>
#include <unordered_set>

#include "BasicBlock.hpp"
#include "ir/ConditionalJump.hpp"
#include "ir/Jump.hpp"

std::pair<jl::BasicBlock*, jl::BasicBlock*> jl::algorithms::get_successors(BasicBlock* block)
{
    const auto terminator = block->get_terminator();

    if (auto jump = dynamic_cast<const ir::Jump*>(terminator)) {
        return { jump->m_target, nullptr };
    } else if (auto cond_jump = dynamic_cast<const ir::CondJump*>(terminator)) {
        return { cond_jump->m_true_target, cond_jump->m_false_target };
    } else {
        return { nullptr, nullptr };
    }
}

std::unordered_map<jl::BasicBlock*, std::vector<jl::BasicBlock*>> jl::algorithms::get_predecessors(Function* function)
{
    std::unordered_map<BasicBlock*, std::vector<BasicBlock*>> preds;

    for (auto& block : function->blocks()) {
        auto [l, r] = get_successors(block.get());
        if (l != nullptr)
            preds[l].push_back(block.get());
        if (r != nullptr)
            preds[r].push_back(block.get());
    }

    return preds;
}

std::unordered_map<jl::BasicBlock*, uint32_t> jl::algorithms::RPO(BasicBlock* entry_block)
{
    std::unordered_set<BasicBlock*> visited;
    std::stack<BasicBlock*> stk;
    std::vector<BasicBlock*> post_order;

    if (entry_block) {
        stk.push(entry_block);
        visited.insert(entry_block);
    }

    while (!stk.empty()) {
        auto node = stk.top();

        auto [left, right] = jl::algorithms::get_successors(node);

        // Find an unvisited child to descend into
        bool pushed = false;

        if (left && !visited.contains(left)) {
            visited.insert(left);
            stk.push(left);
            pushed = true;
        }
        if (right && !visited.contains(right)) {
            visited.insert(right);
            stk.push(right);
            pushed = true;
        }

        if (!pushed) {
            // All children processed — emit this node
            post_order.push_back(node);
            stk.pop();
        }
    }

    std::unordered_map<jl::BasicBlock*, uint32_t> rpo;
    uint32_t count = 0;

    for (auto it = post_order.crbegin(); it != post_order.crend(); ++it) {
        rpo[*it] = count++;
    }

    return rpo;
}
