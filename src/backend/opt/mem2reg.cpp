#include "BasicBlock.hpp"
#include "Function.hpp"
#include "Optimizer.hpp"

#include "Utils.hpp"
#include "ir/AllocateVar.hpp"
#include "ir/ConditionalJump.hpp"
#include "ir/IR.hpp"
#include "ir/Jump.hpp"
#include "ir/Phi.hpp"
#include "ir/Read.hpp"
#include "ir/Write.hpp"
#include "utils/algorithms.hpp"
#include "value/Variable.hpp"
#include <memory>
#include <optional>
#include <print>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

bool is_promotable(jl::ir::AllocateVar* alloca, jl::Function* parent)
{
    for (auto& block : parent->blocks()) {
        for (auto ir = block->head; ir != nullptr; ir = ir->next) {
            if (!ir->uses(alloca->m_addr))
                continue;

            // If a write uses
            if (dynamic_cast<jl::ir::Read*>(ir)) {
                continue;
            }

            if (auto write = dynamic_cast<jl::ir::Write*>(ir)) {
                if (write->m_src.id() != alloca->m_addr.id()) {
                    continue;
                }
            }

            return false;
        }
    }

    return true;
}

struct PhiPlaceHolder {
    jl::ir::AllocateVar* alloca;
};

using PhiInBB = std::unordered_map<jl::BasicBlock*, std::vector<PhiPlaceHolder>>;

void insert_phi_instrs(jl::ir::AllocateVar* alloca,
    jl::Function* function,
    const jl::algorithms::DominanceFrontier& df)
{
    std::vector<jl::BasicBlock*> blocks_with_writes;
    std::unordered_set<jl::BasicBlock*> phi_added_blocks;

    // std::println("Processing {}", alloca->to_str());

    // Find all the blocks with writes where the alloca is used
    for (auto& block : function->blocks()) {
        const auto instrs = block->get_instrs<jl::ir::Write*>();
        for (auto instr : instrs) {
            if (instr->uses(alloca->m_addr)) {
                blocks_with_writes.push_back(block.get());
                // std::println("\tadding block {} with write: {}", block->get_name(), instr->to_str());
            }
        }
    }

    while (!blocks_with_writes.empty()) {
        jl::BasicBlock* block = blocks_with_writes.back();
        blocks_with_writes.pop_back();

        // std::println("- taking block: {}", block->get_name());

        if (!df.contains(block))
            continue;

        for (auto frontier : df.at(block)) {
            if (phi_added_blocks.contains(frontier)) {
                continue;
            }

            // std::println("\tfrontier: {}", frontier->get_name());

            phi_added_blocks.insert(frontier);
            blocks_with_writes.push_back(frontier);

            // Insert phi instr
            auto var = jl::value::Variable(function->m_var_count++, alloca->m_var_type);
            auto phi = new jl::ir::Phi(var, alloca->m_addr);
            function->irs().emplace_back(phi);
            frontier->phis.push_back(phi);
            phi->parent = frontier;
        }
    }
}

// maps r/w addr Variable -> (Basicblock, current Variable)
using VariableValue = std::vector<std::unordered_map<jl::value::Variable, std::pair<jl::BasicBlock*, jl::value::Variable>>>;

std::optional<std::pair<jl::BasicBlock*, jl::value::Variable>> get_current_value(VariableValue& variable_values, jl::value::Variable addr)
{
    for (auto it = variable_values.crbegin(); it != variable_values.crend(); ++it) {
        if (it->contains(addr)) {
            return it->at(addr);
        }
    }

    return std::nullopt;
};

// Helper function to replace a value present in both the the variable stack and function
void replace_value(
    jl::Function* function,
    VariableValue& variable_values,
    jl::value::Variable from,
    jl::value::Variable to)
{
    function->replace_value(from, to);

    for (auto& map : variable_values) {
        for (auto it = map.begin(); it != map.end(); ++it) {

            if (it->second.second == from) {
                it->second.second = to;
                // std::println("Original: {}, After mod: {}", from.to_str(), map.at(it->first).second.to_str());
            }
        }
    }
}

void remove_read_and_writes(
    jl::BasicBlock* block,
    jl::Function* function,
    VariableValue& variable_values,
    std::unordered_set<jl::BasicBlock*> visited)
{
    // std::println("Starting Block - {}", block->get_name());

    // Set the block and value for each of the phis
    for (auto phi : block->phis) {
        if (auto value = get_current_value(variable_values, phi->m_replacing_addr)) {
            auto [blk, ssa] = *value;
            // std::println("- Inserted value ({}, {}) into phi - {}", blk->get_name(), ssa.to_str(), phi->to_str());

            phi->m_opers.insert({ ssa, blk });
            // Now any query for the alloca address will be returned with phi's value
            variable_values.back().insert({ phi->m_replacing_addr, { block, phi->m_dest } });
        } else {
            unimplemented("Should not reach here");
        }
    }

    if (visited.contains(block)) {
        return;
    }

    visited.insert(block);

    std::vector<jl::ir::IR*> to_be_removed;

    for (auto instr = block->head; instr != nullptr; instr = instr->next) {
        if (auto read = dynamic_cast<jl::ir::Read*>(instr)) {
            auto [blk, ssa] = *get_current_value(variable_values, read->m_base);
            // function->replace_value(read->m_dest, ssa);

            replace_value(function, variable_values, read->m_dest, ssa);
            to_be_removed.push_back(read);

            // std::println("\tRead - replacing {} with {}", read->m_dest.to_str(), ssa.to_str());
        } else if (auto write = dynamic_cast<jl::ir::Write*>(instr)) {
            variable_values.back().insert({ write->m_base, { block, write->m_src } });
            to_be_removed.push_back(write);

            // std::println("\tWrite - removing {}", write->to_str());
        } else if (auto branch = dynamic_cast<jl::ir::CondJump*>(instr)) {
            // std::println("[To True Branch] {}", branch->m_true_target->get_name());

            variable_values.push_back({});
            remove_read_and_writes(branch->m_true_target, function, variable_values, visited);
            variable_values.pop_back();

            // std::println("[To False Branch] {}", branch->m_false_target->get_name());

            variable_values.push_back({});
            remove_read_and_writes(branch->m_false_target, function, variable_values, visited);
            variable_values.pop_back();
        } else if (auto jmp = dynamic_cast<jl::ir::Jump*>(instr)) {
            // std::println("[To Branch] {}", jmp->m_target->get_name());

            variable_values.push_back({});
            remove_read_and_writes(jmp->m_target, function, variable_values, visited);
            variable_values.pop_back();
        }
    }

    // std::println("Ending Block - {}", block->get_name());

    for (auto ir : to_be_removed) {
        // block->remove_ir(ir);
        function->remove_ir(block, ir);
    }
}

void jl::opt::mem2reg(Function* function)
{
    std::vector<ir::AllocateVar*> allocas;
    auto entry_block = function->entry_block();

    for (auto ir = entry_block->head; ir != nullptr; ir = ir->next) {
        if (auto alloca = dynamic_cast<ir::AllocateVar*>(ir)) {
            if (is_promotable(alloca, function)) {
                allocas.push_back(alloca);
            }
        }
    }

    auto rpo_map = jl::algorithms::RPO(function->entry_block());
    std::vector<jl::BasicBlock*> rpo(rpo_map.size(), nullptr);
    for (auto& [block, idx] : rpo_map) {
        rpo[idx] = block;
    }

    auto DF = algorithms::dominance_frontier(function, rpo);

    // Put place holder phis for each alloca in every frontier block
    for (auto alloca : allocas) {
        insert_phi_instrs(alloca, function, DF);
    }

    // std::println("=========Phis are placed==============");

    VariableValue variable_values;
    std::unordered_set<jl::BasicBlock*> visited;

    variable_values.push_back({});
    remove_read_and_writes(function->entry_block(), function, variable_values, visited);

    for (auto alloca : allocas) {
        // entry_block->remove_ir(alloca);
        function->remove_ir(entry_block, alloca);
    }
}
