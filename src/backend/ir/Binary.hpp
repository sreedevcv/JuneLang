#pragma once

#include "backend/ir/IR.hpp"
#include "backend/value/Variable.hpp"

#include <cstdint>

namespace jl {
namespace ir {
    struct Binary : public IR {
        enum Operation {
            PLUS,
            MINUS,
            STAR,
            SLASH,
            GREATER,
            LESS,
            GREATER_EQUAL,
            LESS_EQUAL,
            EQUAL_EQUAL,
            BANG_EQUAL,
            PERCENT,
            BIT_AND,
            BIT_OR,
            BIT_XOR,
            LOG_AND,
            LOG_OR,
        };

        Binary(
            value::Variable dest,
            value::Variable operand_a,
            value::Variable operand_b,
            Operation operation,
            bool is_float,
            uint32_t line);

        ~Binary() = default;

        std::string to_str() const override;

        void accept(IRVisitor& visitor) override;

        bool uses(value::Variable var) override;

        void replace(value::Variable from, value::Variable to) override;

        value::Variable m_dest;
        value::Variable m_operand_a;
        value::Variable m_operand_b;
        Operation m_operation;
        bool m_is_float;
    };
}
}
