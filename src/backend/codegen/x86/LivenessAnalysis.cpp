#include "codegen/x86/Passes.hpp"

#include "codegen/x86/Instruction.hpp"
#include "codegen/x86/MachineBlock.hpp"
#include "codegen/x86/MachineFunction.hpp"
#include "codegen/x86/Register.hpp"
#include <cstdint>
#include <print>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

struct LivenessAnalysis {
    jl::x86::MachineFunction* function;
    std::vector<jl::x86::MachineBlock*> rpo;

    using reg_set = std::unordered_set<jl::x86::VirtualRegister, jl::x86::VirtualRegisterHasher>;

    std::unordered_map<jl::x86::MachineBlock*, reg_set> gen; /* uses */
    std::unordered_map<jl::x86::MachineBlock*, reg_set> kill; /* defs */
    std::unordered_map<jl::x86::MachineBlock*, reg_set> live_in;
    std::unordered_map<jl::x86::MachineBlock*, reg_set> live_out;
    std::unordered_map<jl::x86::MachineBlock*, std::vector<jl::x86::MachineBlock*>> successors;

    LivenessAnalysis(jl::x86::MachineFunction* function)
        : function(function)
        , rpo(function->rpo())
    {
    }

    void number_instructions()
    {
        uint32_t instr_id = 0;

        for (auto block : rpo) {
            for (auto& instr : block->m_instructions) {
                instr->m_id = instr_id;
                instr_id += 1;
            }
        }
    }

    void liveness_analysis()
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
                auto old_size = live_out[block].size();
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
};

void jl::x86::pass::liveness_analysis(jl::x86::MachineFunction* function)
{
    LivenessAnalysis la(function);
    la.liveness_analysis();

    std::println("~~~~~~~~~~~~~~~~~~Liveness Analysis~~~~~~~~~~~~~~~~~~~~~~");

    for (const auto& block : function->blocks()) {
        std::println("Block {}", block->m_name);
        std::print("Live in: ");

        for (auto vr : la.live_in[block.get()]) {
            std::print("{}, ", vr.to_string());
        }

        std::print("\nLive out: ");
        for (auto vr : la.live_out[block.get()]) {
            std::print("{}, ", vr.to_string());
        }

        std::println("\n{}", block->to_string());
    }
}
