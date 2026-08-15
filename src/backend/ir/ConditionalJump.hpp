#pragma once

#include "backend/ir/IR.hpp"
#include "value/Variable.hpp"
#include <cstdint>

namespace jl {
class BasicBlock;

namespace ir {
    struct CondJump : public IR {
        CondJump(value::Variable condition,
            BasicBlock* true_target,
            BasicBlock* false_target,
            uint32_t line);

        std::string to_str() const override;

        void accept(IRVisitor& visitor) override;

        bool is_used(value::Variable var) override;

        std::vector<value::Variable> uses() const override;

        void replace(value::Variable from, value::Variable to) override;

        std::optional<jl::value::Variable> def() override;

        BasicBlock* m_true_target;
        BasicBlock* m_false_target;
        value::Variable m_condition;
    };
}
}
