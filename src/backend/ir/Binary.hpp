#pragma once

#include "backend/ir/IR.hpp"
#include "backend/value/Variable.hpp"

#include <cstdint>
#include <memory>

namespace jl {
namespace ir {
    class Binary : public IR {
    public:
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
        };

        Binary(
            std::shared_ptr<value::Variable> dest,
            std::shared_ptr<value::Variable> operand_a,
            std::shared_ptr<value::Variable> operand_b,
            Operation operation,
            uint32_t line);

        virtual ~Binary() = default;

        virtual std::string to_str() const = 0;

    private:
        std::shared_ptr<value::Variable> m_dest;
        std::shared_ptr<value::Variable> m_operand_a;
        std::shared_ptr<value::Variable> m_operand_b;
        Operation m_operation;
    };
}
}
