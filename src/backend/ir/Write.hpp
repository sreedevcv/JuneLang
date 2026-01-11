#pragma once

#include "ir/IR.hpp"
#include "value/Variable.hpp"
#include <cstdint>

namespace jl {
namespace ir {
    struct Write : public IR {
        Write(
            value::Variable src,
            value::Variable base,
            value::Variable offset,
            uint32_t offset_multiplier,
            uint32_t size,
            uint32_t line);

        ~Write() = default;

        std::string to_str() const override;

        void accept(IRVisitor& visitor) override;

        value::Variable m_src;
        value::Variable m_base;
        value::Variable m_offset;
        uint32_t m_offset_multiplier;
        uint32_t m_size;
    };
}
}
