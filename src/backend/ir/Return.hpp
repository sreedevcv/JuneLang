#pragma once

#include "backend/ir/IR.hpp"
#include "backend/value/Variable.hpp"

#include <cstdint>
#include <memory>
#include <optional>

namespace jl {
namespace ir {
    class Return : public IR {
    public:
        Return(std::optional<std::shared_ptr<value::Variable>> ret_val, uint32_t line);

        ~Return() = default;

        std::string to_str() const override;

    private:
        std::optional<std::shared_ptr<value::Variable>> m_ret_val;
    };
}
}
