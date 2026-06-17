#pragma once

#include "backend/ir/IR.hpp"
#include "backend/value/Variable.hpp"
#include "types/Type.hpp"

#include <cstdint>

namespace jl {
namespace ir {
    struct TypeCast : public IR {
        TypeCast(
            const type::Type* from,
            const type::Type* to,
            value::Variable dest,
            value::Variable source,
            uint32_t line);

        virtual ~TypeCast() = default;

        std::string to_str() const override;

        void accept(IRVisitor& visitor) override;

        bool uses(value::Variable var) override;

        void replace(value::Variable from, value::Variable to) override;

        value::Variable m_dest;
        value::Variable m_source;

        const type::Type* m_from;
        const type::Type* m_to;
    };
}
}
