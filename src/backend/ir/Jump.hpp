#pragma once

#include "backend/ir/IR.hpp"
#include "backend/value/Variable.hpp"
#include <cstdint>
#include <memory>
#include <optional>

namespace jl {
namespace ir {
    struct Jump : public IR {
        Jump(uint32_t label, std::optional<std::shared_ptr<value::Variable>> condition, uint32_t line);

        std::string to_str() const override;

        void accept(IRVisitor& visitor) override;

        uint32_t m_label;
        std::optional<std::shared_ptr<value::Variable>> m_condition;
    };

    struct Label : public IR {
        uint32_t m_value;

        Label(uint32_t value, uint32_t line);

        std::string to_str() const override;

        void accept(IRVisitor& visitor) override;
    };
}
}
