#pragma once

#include "backend/ir/IR.hpp"
#include "backend/value/Variable.hpp"

#include <cstdint>
#include <memory>

namespace jl {
namespace ir {
    struct Unary : public IR {
        enum Operation {
            MINUS,
            BANG,
            BIT_NOT,
        };

        Unary(
            value::Variable dest,
            value::Variable operand,
            Operation operation,
            uint32_t line);

        ~Unary() = default;

        std::string to_str() const override;

        void accept(IRVisitor& visitor) override;

        value::Variable m_dest;
        value::Variable m_operand;
        Operation m_operation;
    };
}
}
