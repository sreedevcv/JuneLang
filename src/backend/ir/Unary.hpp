#pragma once

#include "backend/ir/IR.hpp"
#include "backend/value/Variable.hpp"

#include <cstdint>
#include <memory>

namespace jl {
namespace ir {
    class Unary : public IR {
    public:
        enum Operation {
            MINUS,
            BANG,
            BIT_NOT,
        };

        Unary(
            std::shared_ptr<value::Variable> dest,
            std::shared_ptr<value::Variable> operand,
            Operation operation,
            uint32_t line);

        ~Unary() = default;

        std::string to_str() const;

    private:
        std::shared_ptr<value::Variable> m_dest;
        std::shared_ptr<value::Variable> m_operand;
        Operation m_operation;
    };
}
}
