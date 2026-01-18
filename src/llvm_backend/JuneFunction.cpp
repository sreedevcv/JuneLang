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

void jl::JuneFunction::add_local_var_def(const std::string& name, llvm::Value* value, llvm::Type* type)
{
    m_scopes.back().defs[name] = { value, type };
}

std::pair<llvm::Value*, llvm::Type*> jl::JuneFunction::read_local_var_def(const std::string& name)
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
