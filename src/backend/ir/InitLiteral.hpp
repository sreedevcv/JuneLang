#pragma once

#include "backend/LiteralValue.hpp"
#include "backend/ir/IR.hpp"
#include "backend/value/Variable.hpp"

namespace jl {
namespace ir {
    struct InitLiteral : public IR {
        InitLiteral(LiteralValue source,
            value::Variable dest,
            uint32_t line);

        virtual ~InitLiteral() = default;

        void accept(IRVisitor& visitor) override;

        bool is_used(value::Variable var) override;

        std::vector<value::Variable> uses() const override;

        void replace(value::Variable from, value::Variable to) override;

        std::optional<jl::value::Variable> def() override;

        std::string to_str() const override;
        LiteralValue m_source;
        value::Variable m_dest;
    };
}
}
