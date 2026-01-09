#pragma once

#include "backend/ir/IR.hpp"
#include "backend/value/Variable.hpp"

#include <cstdint>
#include <memory>
#include <optional>

namespace jl {
namespace ir {
    struct Return : public IR {
        Return(std::optional<std::shared_ptr<value::Variable>> ret_val, uint32_t line);

        ~Return() = default;

        std::string to_str() const override;

        void accept(IRVisitor& visitor) override;

        std::optional<std::shared_ptr<value::Variable>> m_ret_val;
    };
}
}
