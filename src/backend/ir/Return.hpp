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

        std::optional<value::Variable> m_ret_val;
    };
}
}
