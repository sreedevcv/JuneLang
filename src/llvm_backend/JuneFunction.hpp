#pragma once

#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/ValueHandle.h>
#include <string>
#include <unordered_map>
#include <utility>

namespace jl {

class JuneFunction {
public:
    JuneFunction(std::string name);

    struct VarDef {
        std::unordered_map<std::string, std::pair<llvm::TrackingVH<llvm::Value>, llvm::Type*>> defs;
        llvm::DenseMap<llvm::PHINode*, const std::string*> incomplete_phis;
        bool sealed;
    };

    void set_current_basic_block(llvm::BasicBlock* basic_block);

    void add_local_var_definition(const std::string& name, llvm::Value* value, llvm::Type* type);

    std::pair<llvm::Value*, llvm::Type*> read_local_var_definiton(const std::string& name);

    void push_scope();

    void pop_scope();

private:
    std::string m_name;
    llvm::DenseMap<llvm::BasicBlock*, VarDef> m_var_def_map;
    llvm::BasicBlock* m_current_bb;
    llvm::SmallVector<VarDef, 5> m_scopes;

    void add_local_var_definition(const std::string& name, llvm::Value* value, llvm::BasicBlock* block);

    llvm::Value* read_local_var_definiton(const std::string& name, llvm::BasicBlock* block);

    llvm::Value* read_local_var_defintion_recursive(llvm::BasicBlock* block, const std::string& name);
};
}
