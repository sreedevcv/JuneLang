#pragma once

#include "backend/ir/IR.hpp"
#include <cstdint>

namespace jl {
class BasicBlock;

namespace ir {
    struct Jump : public IR {
        Jump(BasicBlock* target, uint32_t line);

        std::string to_str() const override;

        void accept(IRVisitor& visitor) override;

        bool uses(value::Variable var) override;

        void replace(value::Variable from, value::Variable to) override;

        BasicBlock* m_target;
    };
}
}
