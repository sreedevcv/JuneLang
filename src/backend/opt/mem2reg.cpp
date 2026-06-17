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
#include "utils/UseIter.hpp"
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
        jl::util::UseIter uses(block.get(), alloca->m_addr);

        while (uses.has_next()) {
            auto ir = uses.next();

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

    // Find all the blocks with writes where the alloca is used
    for (auto& block : function->blocks()) {
        const auto instrs = block->get_instrs<jl::ir::Write*>();
        for (auto instr : instrs) {
            if (instr->uses(alloca->m_addr)) {
                blocks_with_writes.push_back(block.get());
                continue;
            }
        }
    }

    while (!blocks_with_writes.empty()) {
        jl::BasicBlock* block = blocks_with_writes.back();
        blocks_with_writes.pop_back();

        for (auto frontier : df.at(block)) {
            if (phi_added_blocks.contains(frontier)) {
                continue;
            }

            phi_added_blocks.insert(frontier);
            blocks_with_writes.push_back(frontier);

            // Insert phi instr
            auto var = jl::value::Variable(function->m_var_count++, alloca->m_var_type);
            auto phi = new jl::ir::Phi(var, alloca->m_addr);
            function->irs().emplace_back(phi);
            block->phis.push_back(phi);
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

void remove_read_and_writes(
    jl::BasicBlock* block,
    jl::Function* function,
    VariableValue& variable_values,
    std::unordered_set<jl::BasicBlock*> visited)
{

    // Set the block and value for each of the phis
    for (auto phi : block->phis) {
        if (auto value = get_current_value(variable_values, phi->m_replacing_addr)) {
            auto [blk, ssa] = *value;
            phi->m_opers.push_back({ ssa, blk });
        } else {
            unimplemented("Should not reach here");
        }
    }

    if (visited.contains(block)) {
        return;
    }

    visited.insert(block);

    std::vector<jl::ir::IR*> to_be_removed;

    for (auto& block : function->blocks()) {
        for (auto instr = block->head; instr != nullptr; instr = instr->next) {
            if (auto read = dynamic_cast<jl::ir::Read*>(instr)) {
                auto [blk, ssa] = *get_current_value(variable_values, read->m_base);
                function->replace_value(read->m_dest, ssa);
            }
            if (auto write = dynamic_cast<jl::ir::Write*>(instr)) {
                variable_values.back().insert({ write->m_base, { block.get(), write->m_src } });
                to_be_removed.push_back(write);
            }
            if (auto branch = dynamic_cast<jl::ir::CondJump*>(instr)) {
            }
            if (auto jmp = dynamic_cast<jl::ir::Jump*>(instr)) {
            }
        }
    }
}

void jl::opt::mem2reg(Function* function)
{
    std::vector<ir::AllocateVar*> allocas;
    auto entry_block = function->entry_block();

    for (auto ir = entry_block->head; ir != nullptr; ir = ir->next) {
        if (auto alloca = dynamic_cast<ir::AllocateVar*>(ir)) {
            std::println("\t{}", alloca->to_str());
            if (is_promotable(alloca, function)) {
                allocas.push_back(alloca);
            }
        }
        std::println("{}", ir->to_str());
    }
    std::println("DONE");

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

    VariableValue variable_values;
    std::unordered_set<jl::BasicBlock*> visited;

    // variable_values.push_back({});
    // remove_read_and_writes(function->entry_block(), function, variable_values, visited);
}
