#pragma once

#include "TypeInfo.hpp"
#include <memory>
#include <string>
#include <vector>

namespace jl {
namespace type {
    struct Type {
        enum Kind {
            BUILTIN,
            PTR,
            FUNC
        };

        Kind m_kind;

        Type(Kind kind);
        virtual ~Type() = default;
        virtual bool equals(const Type* type) const = 0;
        virtual std::string to_str() const = 0;
        virtual std::unique_ptr<Type> clone() const = 0;
    };

    struct Builtin : Type {
        enum Primitive {
            INT,
            FLOAT,
            BOOL,
            CHAR,
        };

        Primitive m_primitive;

        Builtin(Primitive primitive);
        virtual ~Builtin() = default;

        bool equals(const Type* type) const override;
        std::string to_str() const override;
        std::unique_ptr<Type> clone() const override;
    };

    struct Pointer : Type {
        std::unique_ptr<Type> m_pointee;

        Pointer(std::unique_ptr<Type> pointee);
        virtual ~Pointer() = default;

        bool equals(const Type* type) const override;
        std::string to_str() const override;
        std::unique_ptr<Type> clone() const override;
    };

    struct Func : Type {
        std::unique_ptr<Type> m_out;
        std::vector<std::unique_ptr<Type>> m_in;

        Func(std::unique_ptr<Type> out, std::vector<std::unique_ptr<Type>> in);
        virtual ~Func() = default;

        bool equals(const Type* type) const override;
        std::string to_str() const override;
        std::unique_ptr<Type> clone() const override;
    };

    bool is_number(const Type* t);
    std::optional<std::unique_ptr<Type>> from_type_info(const TypeInfo& type_info);
    bool is_boolean(const jl::type::Type* t);
}
}
