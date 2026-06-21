#pragma once

#include "backend/ir/IR.hpp"
#include "backend/value/Variable.hpp"
#include <cstdint>
#include <vector>

namespace jl {
namespace ir {
    struct Call : public IR {
        Call(const std::string& name,
            std::vector<value::Variable> args,
            value::Variable dest,
            uint32_t line);

        ~Call() = default;

        std::string to_str() const override;

        void accept(IRVisitor& visitor) override;

        bool uses(value::Variable var) override;

        void replace(value::Variable from, value::Variable to) override;

        std::optional<jl::value::Variable> def() override;

        std::string m_name;
        std::vector<value::Variable> m_args;
        value::Variable m_dest;
    };
}
}
