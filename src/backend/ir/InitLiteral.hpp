#pragma once

#include "backend/LiteralValue.hpp"
#include "backend/ir/IR.hpp"
#include "backend/value/Variable.hpp"
#include <memory>

namespace jl {
namespace ir {
    struct InitLiteral : public IR {
        InitLiteral(std::unique_ptr<LiteralValue> source,
            value::Variable dest,
            uint32_t line);

        virtual ~InitLiteral() = default;

        void accept(IRVisitor& visitor) override;

        bool uses(value::Variable var) override;

        void replace(value::Variable from, value::Variable to) override;

        std::string to_str() const override;
        std::unique_ptr<LiteralValue> m_source;
        value::Variable m_dest;
    };
}
}
