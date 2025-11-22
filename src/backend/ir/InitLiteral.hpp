#pragma once

#include "backend/LiteralValue.hpp"
#include "backend/ir/IR.hpp"
#include "backend/value/Variable.hpp"
#include <memory>

namespace jl {
namespace ir {
    class InitLiteral : public IR {
    public:
        InitLiteral(std::unique_ptr<LiteralValue> source,
            std::shared_ptr<value::Variable> dest,
            uint32_t line);

    private:
        std::unique_ptr<LiteralValue> m_source;
        std::shared_ptr<value::Variable> m_dest;
    };
}
}
