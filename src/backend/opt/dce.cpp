#include "opt/Optimizer.hpp"

#include "Function.hpp"
#include "ir/ConditionalJump.hpp"
#include "ir/InitLiteral.hpp"
#include "ir/Jump.hpp"
#include "ir/Phi.hpp"
#include "utils/algorithms.hpp"
#include <stack>

// Remove unexecuted blocks and all reference to them from the CFG
void remove_unexecuted_blocks(jl::Function* function, const jl::opt::ExecMap& exec_map)
{
    std::vector<jl::BasicBlock*> blocks_to_be_deleted;
    std::unordered_map<jl::BasicBlock*, uint32_t> in_edges;

    for (auto& block : function->blocks()) {
        in_edges[block.get()] = 0;
    }

    for (auto [edge, flag] : exec_map) {
        if (flag) {
            in_edges[edge.second] += 1;
        }
    }

    auto predecessors = jl::algorithms::get_predecessors(function);

    for (auto [block, val] : in_edges) {
        if (val > 0) {
            continue;
        }

        // Mark the block to be deleted later
        blocks_to_be_deleted.push_back(block);

        // If an predecessors of the block have a conditional jump to this block,
        // then change it a unconditional jump and remove the reference this to block
        for (auto pred : predecessors[block]) {
            auto terminator = pred->get_terminator();

            if (auto jump = dynamic_cast<jl::ir::CondJump*>(terminator)) {
                auto [succ1, succ2] = jl::algorithms::get_successors(pred);
                auto remaining_target = jump->m_true_target == block ? jump->m_false_target : jump->m_true_target;
                assert(remaining_target != nullptr && "atleast one live target to jump to");
                function->remove_ir(jump);
                function->set_current_block(pred);
                function->add_ir(jl::ir::Jump(remaining_target, jump->m_line));
            }
        }

        // If this block is being used by a phi node, then remove the block from its
        // list of operands
        for (auto& blk : function->blocks()) {
            for (auto phi : blk->phis) {
                auto iter = std::find_if(phi->m_opers.begin(),
                    phi->m_opers.end(),
                    [&block](auto&& pair) { return pair.second == block; });

                if (iter != phi->m_opers.end()) {
                    phi->m_opers.erase(iter);
                }
            }
        }
    }

    for (auto block : blocks_to_be_deleted) {
        // std::println("Deleting {}", block->get_name());
        function->remove_block(block);
    }
}

// Removes all defs that have a CONSTAT lattice value since
// we now know what its value is
std::unordered_map<jl::value::Variable, jl::ir::IR*> remove_constant_defs(jl::Function* function, const jl::opt::ValueMap& lattice_values)
{
    std::unordered_map<jl::value::Variable, jl::ir::IR*> to_be_removed;

    constexpr auto is_used = [](jl::Function* function, jl::value::Variable def) {
        for (auto& ir : function->irs()) {
            if (ir->is_used(def)) {
                return true;
            }
        }

        return false;
    };

    for (auto& ir : function->irs()) {
        if (auto def = ir->def()) {

            if (lattice_values.at(*def).type != jl::opt::CONSTANT) {
                if (is_used(function, *def)) {
                    continue;
                }
            }

            to_be_removed[*def] = ir.get();
        }
    }

    return to_be_removed;
}

void add_literal_ir_if_constant(const jl::value::Variable& var,
    std::unordered_map<jl::value::Variable, jl::ir::IR*>& to_be_removed,
    std::unordered_set<uint32_t>& added,
    std::vector<jl::ir::InitLiteral>& new_literals,
    const jl::opt::ValueMap& lattice_values,
    jl::Function* function)
{
    // Add this only if its a constant and has not already been added
    if (lattice_values.at(var).type == jl::opt::CONSTANT && !added.contains(var.id())) {
        // If the variable is already marked for removal then remove it from the list
        // Check if its a constant literal, if its not then replace it with one
        if (to_be_removed.contains(var)) {
            auto ir = to_be_removed[var];
            if (!dynamic_cast<jl::ir::InitLiteral*>(ir)) {
                // This is not constant literal so we replace this ir with a constant literal
                auto literal = jl::LiteralValue(*lattice_values.at(var).value);
                auto init_literal = new jl::ir::InitLiteral(std::move(literal), var, 0);
                function->irs().emplace_back(init_literal);
                function->replace_ir(ir, init_literal);
            }
            // Already present, but marked for deletion, so remove it from the to_be_removed list
            to_be_removed.erase(var);
        } else {
            // Add as a new literal
            auto literal = jl::LiteralValue(*lattice_values.at(var).value);
            new_literals.emplace_back(std::move(literal), var, 0);
        }
        added.insert(var.id());
    }
}

// Add a constant literal for all uses which has a constant value
void add_used_constants_as_literals(std::unordered_map<jl::value::Variable, jl::ir::IR*>& to_be_removed,
    const jl::opt::ValueMap& lattice_values,
    jl::Function* function)
{
    std::unordered_set<uint32_t> added;
    std::vector<jl::ir::InitLiteral> new_literals;

    for (auto& ir : function->irs()) {
        if (auto def = ir->def()) {
            // Only consider those instructions which have not been marked for removal
            if (to_be_removed.contains(*def)) {
                continue;
            }
        }

        const auto uses = ir->uses();
        for (const auto use : uses) {
            add_literal_ir_if_constant(use, to_be_removed, added, new_literals, lattice_values, function);
        }
    }

    auto entry = function->entry_block();
    auto terminator = entry->get_terminator();
    function->set_current_block(entry);

    for (auto literal : new_literals) {
        auto var = literal.m_dest;

        // If the variable is already marked for removal then remove it from the list
        // Check if its a constant literal, if its not then replace it with one
        if (to_be_removed.contains(var)) {
            auto ir = to_be_removed[var];
            if (!dynamic_cast<jl::ir::InitLiteral*>(ir)) {
                // This is not constant literal so we replace this ir with a constant literal
                auto literal = jl::LiteralValue(*lattice_values.at(var).value);
                auto init_literal = new jl::ir::InitLiteral(std::move(literal), var, 0);
                function->irs().emplace_back(init_literal);
                function->replace_ir(ir, init_literal);
            }
            to_be_removed.erase(var);
        } else {
            // Add as a new literal
            function->add_ir_to_front(std::move(literal));
        }
    }

    for (auto [_, ir] : to_be_removed) {
        // std::println("Removing ir: {}", ir->to_str());
        function->remove_ir(ir);
    }
}

// Check if any of the phi operators depend on this block
bool check_if_successors_depend_on(jl::BasicBlock* block)
{
    auto [left, right] = jl::algorithms::get_successors(block);

    if (left != nullptr) {
        for (auto phis : left->phis) {
            for (auto& [var, blk] : phis->m_opers) {
                if (blk == block) {
                    return true;
                }
            }
        }
    }

    if (right != nullptr) {
        for (auto phis : right->phis) {
            for (auto& [var, blk] : phis->m_opers) {
                if (blk == block) {
                    return true;
                }
            }
        }
    }

    return false;
}

void collapse_empty_blocks(jl::Function* function)
{
    auto predecessors = jl::algorithms::get_predecessors(function);
    std::unordered_set<jl::BasicBlock*> to_be_removed;
    std::unordered_set<jl::BasicBlock*> visited;
    std::stack<jl::BasicBlock*> stk;
    stk.push(function->entry_block());

    while (!stk.empty()) {
        auto block = stk.top();
        stk.pop();

        if (visited.contains(block)) {
            continue;
        }

        visited.insert(block);

        size_t instr_count = block->phis.size();
        for (auto ir = block->head; ir != nullptr; ir = ir->next) {
            instr_count += 1;
        }

        auto terminator = block->get_terminator();

        // add the blocks to consider next
        if (auto jmp = dynamic_cast<jl::ir::Jump*>(terminator)) {
            stk.push(jmp->m_target);
        } else if (auto cjmp = dynamic_cast<jl::ir::CondJump*>(terminator)) {
            stk.push(cjmp->m_true_target);
            stk.push(cjmp->m_false_target);
        }

        // If the block has more than 1 instruction then it should not be removed
        if (instr_count > 1) {
            continue;
        }

        // if the only remaining instruction is conditional jump or return, then
        // it should not be removed
        auto next_jump = dynamic_cast<jl::ir::Jump*>(terminator);
        if (next_jump == nullptr) {
            continue;
        }

        if (check_if_successors_depend_on(block)) {
            continue;
        }

        // THis block only contains an unconditional jump, so we can safely
        // remove it
        to_be_removed.insert(block);

        // Replace all references to this block from its predecessors
        for (auto pred : predecessors[block]) {
            if (to_be_removed.contains(pred)) {
                continue;
            }

            auto terminator = pred->get_terminator();

            if (auto cjmp = dynamic_cast<jl::ir::CondJump*>(terminator)) {
                if (cjmp->m_true_target == block) {
                    cjmp->m_true_target = next_jump->m_target;
                }
                if (cjmp->m_false_target == block) {
                    cjmp->m_false_target = next_jump->m_target;
                }
            } else if (auto jmp = dynamic_cast<jl::ir::Jump*>(terminator)) {
                jmp->m_target = next_jump->m_target;
            }
        }
    }

    for (auto block : to_be_removed) {
        // std::println("Removing block: {}", block->get_name());
        function->remove_block(block);
    }
}

void jl::opt::dce(Function* function, const ValueMap& lattice_values, const ExecMap& exec_map)
{
    remove_unexecuted_blocks(function, exec_map);
    auto to_be_removed = remove_constant_defs(function, lattice_values);
    add_used_constants_as_literals(to_be_removed, lattice_values, function);
    collapse_empty_blocks(function);
}