#pragma once

#include "ir/IR.hpp"
#include "types/Type.hpp"
#include "value/Variable.hpp"

namespace jl {
namespace ir {
    struct AllocateVar : public IR {
    public:
        AllocateVar(value::Variable addr, const type::Type* var_type, uint32_t line);

        ~AllocateVar() = default;

        std::string to_str() const override;

        void accept(IRVisitor& visitor) override;

        bool uses(value::Variable var) override;

        void replace(value::Variable from, value::Variable to) override;


        value::Variable m_addr;
        const type::Type* m_var_type;
    };
}
}
