#pragma once

#include "ir/IR.hpp"
#include "types/Type.hpp"
#include "value/Variable.hpp"

#include <cstdint>

namespace jl {
namespace ir {
    struct DebugPrint : public IR {
        DebugPrint(value::Variable val, type::Builtin::Primitive primitive, uint32_t line);

        ~DebugPrint() = default;

        std::string to_str() const override;

        void accept(IRVisitor& visitor) override;

        value::Variable m_val;
        type::Builtin::Primitive m_primitive;
    };
}
}
