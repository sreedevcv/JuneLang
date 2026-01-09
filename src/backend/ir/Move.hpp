#pragma once

#include "backend/ir/IR.hpp"
#include "backend/value/Variable.hpp"

#include <cstdint>
#include <memory>

namespace jl {
namespace ir {
    struct Move : public IR {
        Move(std::shared_ptr<value::Variable> source,
            std::shared_ptr<value::Variable> dest,
            uint32_t line);

        virtual ~Move() = default;

        std::string to_str() const override;

        void accept(IRVisitor& visitor) override;

        std::shared_ptr<value::Variable> m_source;
        std::shared_ptr<value::Variable> m_dest;
    };
}
}
