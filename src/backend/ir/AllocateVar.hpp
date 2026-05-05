#pragma once

#include "ir/IR.hpp"
#include "types/Type.hpp"
#include "value/Variable.hpp"

namespace jl {
namespace ir {
    class AllocateVar : public IR {
    public:
        AllocateVar(value::Variable addr, const type::Type* var_type, uint32_t line);

        ~AllocateVar() = default;

        std::string to_str() const override;

        void accept(IRVisitor& visitor) override;

    private:
        value::Variable m_addr;
        const type::Type* m_var_type;
    };
}
}