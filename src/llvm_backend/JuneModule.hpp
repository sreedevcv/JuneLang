#pragma once

#include "TypeInfo.hpp"
#include "llvm_backend/JuneFunction.hpp"
#include <llvm/IR/Attributes.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include <llvm/IR/Type.h>
#include <string>

namespace jl {

class JuneModule {
public:
    JuneModule(const std::string& file_name);

    std::optional<llvm::Type*> map_to_llvm_type(const TypeInfo& type_info);

    llvm::LLVMContext& ctx();

    llvm::Module& module();

    void set_function(llvm::Function* function);

    JuneFunction& function();

    llvm::Function* llvm_function();

    llvm::IRBuilder<>& builder();

    llvm::Value* allocate_in_entry_block(const std::string& name, llvm::Type* type);

private:
    llvm::LLVMContext m_context;
    llvm::Module m_module;
    llvm::Function* m_llvm_function;
    std::unique_ptr<JuneFunction> m_function;
    llvm::IRBuilder<> m_builder;
};
}
