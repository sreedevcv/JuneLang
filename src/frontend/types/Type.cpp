#include "Type.hpp"
#include "Utils.hpp"
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/Casting.h>
#include <string>

llvm::Value* jl::type::Type::llvm_default_value(llvm::LLVMContext& context) const
{
    return nullptr;
}

std::string jl::type::Builtin::to_str() const
{
    switch (m_primitive) {
    case INT:
        return "int";
    case FLOAT:
        return "float";
    case BOOL:
        return "bool";
    case CHAR:
        return "char";
    case VOID:
        return "void";
    default:
        unimplemented();
        break;
    }
    return "Builtin";
}

uint32_t jl::type::Builtin::size() const
{
    switch (m_primitive) {
    case INT:
        return 8;
    case FLOAT:
        return 8;
    case BOOL:
        return 1; // Change this back to 1 or 4
    case CHAR:
        return 1;
    case VOID:
        return 0;
    default:
        unimplemented();
        break;
    }
    return 0;
}

uint32_t jl::type::Builtin::alignment() const
{
    switch (m_primitive) {
    case INT:
        return 8;
    case FLOAT:
        return 8;
    case BOOL:
        return 1;
    case CHAR:
        return 1;
    case VOID:
        return 1;
    case PRIMITIVE_MAX:
        unimplemented();
        return 0;
        break;
    }
}
llvm::Type* jl::type::Builtin::llvm_type(llvm::LLVMContext& context) const
{
    switch (m_primitive) {
    case INT:
        return llvm::dyn_cast<llvm::Type>(llvm::Type::getInt64Ty(context));
    case FLOAT:
        return llvm::dyn_cast<llvm::Type>(llvm::Type::getDoubleTy(context));
    case BOOL:
        return llvm::dyn_cast<llvm::Type>(llvm::Type::getInt1Ty(context));
    case CHAR:
        return llvm::dyn_cast<llvm::Type>(llvm::Type::getInt8Ty(context));
    case VOID:
        return llvm::dyn_cast<llvm::Type>(llvm::Type::getVoidTy(context));
    case PRIMITIVE_MAX:
        unimplemented();
        return nullptr;
    }
}

llvm::Value* jl::type::Builtin::llvm_default_value(llvm::LLVMContext& context) const
{
    auto type = llvm_type(context);

    switch (m_primitive) {
    case INT:
        return llvm::ConstantInt::get(type, 0, true);
    case FLOAT:
        return llvm::ConstantFP::get(type, 0.0);
    case BOOL:
        return llvm::ConstantInt::get(type, 0);
    case CHAR:
        return llvm::ConstantInt::get(type, '\0');
    case VOID:
        return nullptr;
    case PRIMITIVE_MAX:
        unimplemented();
        return nullptr;
    }
}

jl::type::Pointer::Pointer(const Type* pointee)
    : Type(Kind::PTR)
    , m_pointee(pointee)
{
}

std::string jl::type::Pointer::to_str() const
{
    return "*" + m_pointee->to_str();
}

uint32_t jl::type::Pointer::size() const
{
    return 8;
}

uint32_t jl::type::Pointer::alignment() const
{
    return 8;
}

llvm::Type* jl::type::Pointer::llvm_type(llvm::LLVMContext& context) const
{
    unimplemented();
    return nullptr;
}

jl::type::Func::Func(const Type* return_type, std::vector<const Type*> param_types)
    : Type(Kind::FUNC)
    , m_return_type(return_type)
    , m_param_types(param_types)
{
}

std::string jl::type::Func::to_str() const
{
    std::string str = "Fun(";
    for (int i = 0; i < static_cast<int>(m_param_types.size()) - 1; i++) {
        str += m_param_types[i]->to_str() + ", ";
    }

    if (m_param_types.size() > 0) {
        str += m_param_types.back()->to_str();
    }

    str += ") -> " + m_return_type->to_str();

    return str;
}

uint32_t jl::type::Func::size() const
{
    return 0;
}

uint32_t jl::type::Func::alignment() const
{
    return 8;
}

llvm::Type* jl::type::Func::llvm_type(llvm::LLVMContext& context) const
{
    unimplemented();
    return nullptr;
}

jl::type::List::List(const Type* elem_type, uint32_t count)
    : Type(LIST)
    , m_elem_type(elem_type)
    , m_count(count)
{
}

std::string jl::type::List::to_str() const
{
    return "[" + m_elem_type->to_str() + "; " + std::to_string(m_count) + "]";
}

uint32_t jl::type::List::size() const
{
    return 8;
}

uint32_t jl::type::List::alignment() const
{
    return 8;
}

llvm::Type* jl::type::List::llvm_type(llvm::LLVMContext& context) const
{
    return llvm::ArrayType::get(m_elem_type->llvm_type(context), m_count);
}

bool jl::type::is_number(const Type* t)
{
    if (t->m_kind == Type::BUILTIN) {
        auto a = static_cast<const Builtin*>(t);
        return a->m_primitive == Builtin::INT || a->m_primitive == Builtin::FLOAT;
    }

    return false;
}

bool jl::type::is_boolean(const jl::type::Type* t)
{
    if (t->m_kind != type::Type::BUILTIN) {
        return false;
    } else {
        return static_cast<const type::Builtin*>(t)->m_primitive == type::Builtin::BOOL;
    }
}
