#include "JuneFunction.hpp"

#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/CFG.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Value.h>

#include <llvm/ADT/SmallVector.h>

#include <string>

jl::JuneFunction::JuneFunction(std::string name)
    : m_name(name)
{
    VarDef def;
    m_scopes.push_back(std::move(def));
}

void jl::JuneFunction::set_current_basic_block(llvm::BasicBlock* basic_block)
{
    m_current_bb = basic_block;
}

void jl::JuneFunction::add_local_var_definition(const std::string& name, llvm::Value* value, llvm::Type* type)
{
    // add_local_var_definition(name, value, m_current_bb);
    m_scopes.back().defs[name] = { value, type };
}

void jl::JuneFunction::add_local_var_definition(const std::string& name, llvm::Value* value, llvm::BasicBlock* block)
{
    // m_var_def_map[block].defs[&name] = value;
    // m_scopes.back().defs[name] = value;
}

std::pair<llvm::Value*, llvm::Type*> jl::JuneFunction::read_local_var_definiton(const std::string& name)
{
    for (const auto scope : llvm::reverse(m_scopes)) {
        if (scope.defs.contains(name)) {
            return scope.defs.at(name);
        }
    }

    // This should never happen since semantic analysis has already been done
    return {};
}

void jl::JuneFunction::push_scope()
{
    VarDef def;
    m_scopes.push_back(std::move(def));
}

void jl::JuneFunction::pop_scope()
{
    m_scopes.pop_back();
}

llvm::Value* jl::JuneFunction::read_local_var_definiton(const std::string& name, llvm::BasicBlock* block)
{
    // read
}

llvm::Value* jl::JuneFunction::read_local_var_defintion_recursive(llvm::BasicBlock* block, const std::string& name)
{
    llvm::Value* val = nullptr;

    const auto addEmptyPhiNode = [](llvm::BasicBlock* block, const std::string& name) {
        return block->empty()
            ? llvm::PHINode::Create(/* replace with llvm type of var */ nullptr, 0, "", block)
            : llvm::PHINode::Create(/* replace with llvm type of var */ nullptr, 0, "", &block->front());
    };

    if (!m_var_def_map[block].sealed) {
        auto phi_node = addEmptyPhiNode(block, name);
        m_var_def_map[block].incomplete_phis[phi_node] = &name;
        val = phi_node;
    } else if (auto pred = block->getSinglePredecessor()) {
        val = read_local_var_definiton(name, pred);
    } else {
        auto phi_node = addEmptyPhiNode(block, name);
        add_local_var_definition(name, phi_node, block);
        // val =

        for (auto* pred : llvm::predecessors(block)) {
            phi_node->addIncoming(read_local_var_definiton(name, pred), pred);
        }
        // TODO::Optimize this by removing unncessary phi nodes
        val = phi_node;
    }

    add_local_var_definition(name, val, block);
    return val;
}
