#include "JuneModule.hpp"
#include "Utils.hpp"
#include "llvm_backend/JuneFunction.hpp"

#include <llvm/CodeGen/CommandFlags.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Type.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/CodeGen.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/WithColor.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/TargetParser/Triple.h>

#include <memory>
#include <optional>
#include <string>

jl::JuneModule::JuneModule(const std::string& file_name)
    : m_module(file_name, m_context)
    , m_builder(m_context)
{
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    const auto default_target_triple = llvm::sys::getDefaultTargetTriple();
    llvm::Triple target_triple { default_target_triple };

    llvm::WithColor::note(llvm::outs()) << "Using default_target_triple: " << default_target_triple << '\n';

    llvm::TargetOptions target_options;
    std::string error;
    const llvm::Target* target = llvm::TargetRegistry::lookupTarget(target_triple.getTriple(), error);

    if (target == nullptr) {
        llvm::WithColor::error(llvm::errs()) << error;
        return;
    }

    auto target_machine = target->createTargetMachine(
        target_triple.getTriple(),
        "generic",
        "",
        llvm::TargetOptions {},
        std::nullopt);

    m_module.setTargetTriple(llvm::Triple(target_machine->getTargetTriple().getTriple()));
    m_module.setDataLayout(target_machine->createDataLayout());
}

llvm::LLVMContext& jl::JuneModule::ctx()
{
    return m_context;
}

llvm::Module& jl::JuneModule::module()
{
    return m_module;
}

void jl::JuneModule::set_current_function(llvm::Function* function)
{
    m_llvm_function = function;
    m_function = std::make_unique<JuneFunction>(JuneFunction(function->getName().str()));
}

jl::JuneFunction& jl::JuneModule::function()
{
    return *m_function;
}

llvm::IRBuilder<>& jl::JuneModule::builder()
{
    return m_builder;
}

llvm::Function* jl::JuneModule::llvm_function()
{
    return m_llvm_function;
}

std::optional<llvm::Type*> jl::JuneModule::map_to_llvm_type(const TypeInfo& type_info)
{
    if (type_info.is_array) {
        unimplemented("Array types");
    }

    if (type_info.name == "int") {
        return llvm::Type::getInt64Ty(m_context);
    } else if (type_info.name == "float") {
        return llvm::Type::getDoubleTy(m_context);
    } else if (type_info.name == "bool") {
        return llvm::Type::getInt1Ty(m_context);
    } else if (type_info.name == "char") {
        return llvm::Type::getInt8Ty(m_context);
    } else {
        return std::nullopt;
    }
}

llvm::Value* jl::JuneModule::allocate_in_entry_block(const std::string& name, llvm::Type* type)
{
    auto current_block = m_builder.GetInsertBlock();
    auto& entry_block = m_llvm_function->getEntryBlock();
    m_builder.SetInsertPoint(&entry_block, entry_block.begin());

    auto alloca = m_builder.CreateAlloca(type, nullptr, name);
    m_builder.SetInsertPoint(current_block);

    return alloca;
}

void jl::JuneModule::store_function(const std::string& name, llvm::Function* function)
{
    m_function_map[name] = function;
}

std::optional<llvm::Function*> jl::JuneModule::get_function(const std::string& name) const
{
    if (m_function_map.contains(name)) {
        return m_function_map.at(name);
    } else {
        return std::nullopt;
    }
}
