#pragma once

#include "ir/IR.hpp"
#include "value/Variable.hpp"
#include <cstdint>
#include <optional>

namespace jl {
namespace ir {
    struct Read : public IR {
        Read(
            value::Variable dest,
            value::Variable base,
            std::optional<value::Variable> offset,
            uint32_t size,
            uint32_t line);

        ~Read() = default;

        std::string to_str() const override;

        void accept(IRVisitor& visitor) override;

        bool is_used(value::Variable var) override;

        std::vector<value::Variable> uses() const override;

        void replace(value::Variable from, value::Variable to) override;

        std::optional<jl::value::Variable> def() override;

        value::Variable m_dest;
        value::Variable m_base;
        std::optional<value::Variable> m_offset;
        uint32_t m_size;
    };
}
}
