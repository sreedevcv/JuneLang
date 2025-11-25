#pragma once

#include "TypeInfo.hpp"
#include <memory>
#include <optional>
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
            VOID,
        };

        Primitive m_primitive;

        constexpr Builtin(Primitive primitive)
            : Type(Kind::BUILTIN)
            , m_primitive(primitive)
        {
        }
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
        std::unique_ptr<Type> m_return_type;
        std::vector<std::unique_ptr<Type>> m_param_types;

        Func(std::unique_ptr<Type> return_type, std::vector<std::unique_ptr<Type>> in);
        virtual ~Func() = default;

        bool equals(const Type* type) const override;
        std::string to_str() const override;
        std::unique_ptr<Type> clone() const override;
    };

    struct List : Type {
        std::unique_ptr<Type> m_elem_type;
        uint32_t m_count;

        List(std::unique_ptr<Type> melem_type, uint32_t count);
        virtual ~List() = default;
        bool equals(const Type* type) const override;
        std::string to_str() const override;
        std::unique_ptr<Type> clone() const override;
    };

    bool is_number(const Type* t);
    std::optional<std::unique_ptr<Type>> from_type_info(const TypeInfo& type_info);
    bool is_boolean(const jl::type::Type* t);
}
}
