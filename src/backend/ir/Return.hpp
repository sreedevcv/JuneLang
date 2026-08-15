#pragma once

#include "backend/ir/IR.hpp"
#include "backend/value/Variable.hpp"

#include <cstdint>
#include <optional>

namespace jl {
namespace ir {
    struct Return : public IR {
        Return(std::optional<value::Variable> ret_val, uint32_t line);

        ~Return() = default;

        std::string to_str() const override;

        void accept(IRVisitor& visitor) override;

        bool is_used(value::Variable var) override;

        std::vector<value::Variable> uses() const override;

        void replace(value::Variable from, value::Variable to) override;

        std::optional<jl::value::Variable> def() override;

        std::optional<value::Variable> m_ret_val;
    };
}
}
