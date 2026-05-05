#pragma once

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <string>
#include <vector>

namespace jl {
namespace type {
    struct Type {
        enum Kind {
            BUILTIN,
            PTR,
            FUNC,
            LIST,
        };

        Kind m_kind;

        constexpr Type(Kind kind)
            : m_kind(kind)
        {
        }
        virtual ~Type() = default;
        virtual std::string to_str() const = 0;
        virtual uint32_t size() const = 0;
        virtual llvm::Type* llvm_type(llvm::LLVMContext& context) const = 0;
        virtual llvm::Value* llvm_default_value(llvm::LLVMContext& context) const;
    };

    struct Builtin : Type {
        enum Primitive {
            INT,
            FLOAT,
            BOOL,
            CHAR,
            VOID,
            PRIMITIVE_MAX
        };

        Primitive m_primitive;

        constexpr Builtin(Primitive primitive)
            : Type(Kind::BUILTIN)
            , m_primitive(primitive)
        {
        }
        virtual ~Builtin() = default;

        std::string to_str() const override;
        uint32_t size() const override;
        llvm::Type* llvm_type(llvm::LLVMContext& context) const override;
        llvm::Value* llvm_default_value(llvm::LLVMContext& context) const override;
    };

    struct Pointer : Type {
        const Type* m_pointee;

        Pointer(const Type* pointee);
        virtual ~Pointer() = default;

        std::string to_str() const override;
        uint32_t size() const override;
        llvm::Type* llvm_type(llvm::LLVMContext& context) const override;
    };

    struct Func : Type {
        const Type* m_return_type;
        std::vector<const Type*> m_param_types;

        Func(const Type* return_type, std::vector<const Type*> in);
        virtual ~Func() = default;

        std::string to_str() const override;
        uint32_t size() const override;
        llvm::Type* llvm_type(llvm::LLVMContext& context) const override;
    };

    struct List : Type {
        const Type* m_elem_type;
        uint32_t m_count;

        List(const Type* melem_type, uint32_t count);
        virtual ~List() = default;
        std::string to_str() const override;
        uint32_t size() const override;
        llvm::Type* llvm_type(llvm::LLVMContext& context) const override;
    };

    bool is_number(const Type* t);
    bool is_boolean(const jl::type::Type* t);
}
}
