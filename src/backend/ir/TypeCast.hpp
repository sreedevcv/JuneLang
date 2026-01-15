#pragma once

#include "backend/ir/IR.hpp"
#include "types/Type.hpp"
#include "backend/value/Variable.hpp"

#include <cstdint>
#include <memory>

namespace jl {
namespace ir {
    struct TypeCast : public IR {
        TypeCast(
            std::unique_ptr<type::Type> from,
            std::unique_ptr<type::Type> to,
            value::Variable dest,
            value::Variable source,
            uint32_t line);

        virtual ~TypeCast() = default;

        std::string to_str() const override;

        void accept(IRVisitor& visitor) override;

        value::Variable m_dest;
        value::Variable m_source;

        std::unique_ptr<type::Type> m_from;
        std::unique_ptr<type::Type> m_to;
    };
}
}
