#pragma once

#include "ir/IR.hpp"
#include "types/Type.hpp"
#include "value/Variable.hpp"

#include <cstdint>
#include <memory>

namespace jl {
namespace ir {
    struct DebugPrint : public IR {
        DebugPrint(std::shared_ptr<value::Variable> val, type::Builtin::Primitive primitive, uint32_t line);

        ~DebugPrint() = default;

        std::string to_str() const override;

        void accept(IRVisitor& visitor) override;

        std::shared_ptr<value::Variable> m_val;
        type::Builtin::Primitive m_primitive;
    };
}
}
