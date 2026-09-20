#include "codegen/x86/LivenessAnalysis.hpp"

#include "codegen/x86/Instruction.hpp"
#include "codegen/x86/MachineBlock.hpp"
#include "codegen/x86/MachineFunction.hpp"

#include <cstdint>
#include <unordered_map>
#include <utility>
#include <vector>

jl::x86::LivenessAnalysis::LivenessAnalysis(MachineFunction* function)
    : function(function)
    , rpo(function->rpo())
{
}

void jl::x86::LivenessAnalysis::number_instructions()
{
    uint32_t instr_id = 0;

    for (auto block : rpo) {
        for (auto& instr : block->m_instructions) {
            instr->m_id = instr_id;
            instr_id += 1;
        }
    }
}

void jl::x86::LivenessAnalysis::liveness_analysis()
{
    for (const auto& block : function->blocks()) {
        successors[block.get()] = block->successors();

        for (const auto& instr : block->m_instructions) {
            // Add a use to gen if it has not already been defined
            for (const auto use : instr->uses()) {
                if (!kill[block.get()].contains(use)) {
                    gen[block.get()].insert(use);
                }
            }

            for (const auto def : instr->defs()) {
                kill[block.get()].insert(def);
            }
        }
    }

    // live_in[i] = gen[i] v (live_out[i] - kill[i])
    // live_out[i] = V live_in[n], where n is a successor of i
    bool changed = true;

    while (changed) {
        changed = false;

        for (auto block_iter = rpo.rbegin(); block_iter != rpo.rend(); ++block_iter) {
            auto block = *block_iter;
            reg_set new_out;

            for (auto succ : successors[block]) {
                new_out.insert(live_in[succ].begin(), live_in[succ].end());
            }

            reg_set new_in = gen[block];
            for (const auto& r : new_out) {
                if (!kill[block].contains(r)) {
                    new_in.insert(r);
                }
            }

            if (new_out.size() != live_out[block].size() || new_in.size() != live_in[block].size()) {
                changed = true;
            }

            live_in[block] = std::move(new_in);
            live_out[block] = std::move(new_out);
        }
    }
}

jl::x86::LiveIntervalMap jl::x86::LivenessAnalysis::calculate_live_intervals()
{
    number_instructions();

    liveness_analysis();

    jl::x86::LiveIntervalMap intervals;

    for (auto input_param : function->inputs()) {
        intervals[input_param].start = 0;
        intervals[input_param].end = 0;
    }

    const auto succs = function->successors();

    for (const auto& block : rpo) {
        reg_set live;
        auto& succs = successors[block];

        for (auto succ : succs) {
            live.insert(live_in[succ].cbegin(), live_in[succ].cend());
        }

        for (auto var : live) {
            intervals[var].add_range(block->m_instructions.front()->m_id, block->m_instructions.back()->m_id);
        }

        for (auto iter = block->m_instructions.rbegin(); iter != block->m_instructions.rend(); ++iter) {
            auto& ir = *iter;

            for (auto def : ir->defs()) {
                intervals[def].set_start(ir->m_id);
            }

            for (auto use : ir->uses()) {
                intervals[use].add_range(block->m_instructions.front()->m_id, ir->m_id);
            }
        }
    }

    // Store the corresponding virtual registers to each range since
    // we will be using it to hash the range during reg alloc
    for (auto& [reg, range] : intervals) {
        range.vreg = reg;
    }

    return intervals;
}
