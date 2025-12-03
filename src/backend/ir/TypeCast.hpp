#pragma once

#include "backend/ir/IR.hpp"
#include "backend/types/Type.hpp"
#include "backend/value/Variable.hpp"

#include <cstdint>
#include <memory>

namespace jl {
namespace ir {
    class TypeCast : public IR {
    public:
        TypeCast(
            std::unique_ptr<type::Type> from,
            std::unique_ptr<type::Type> to,
            std::shared_ptr<value::Variable> dest,
            std::shared_ptr<value::Variable> source,
            uint32_t line);

        virtual ~TypeCast() = default;

        virtual std::string to_str() const;

    private:
        std::shared_ptr<value::Variable> m_dest;
        std::shared_ptr<value::Variable> m_source;

        std::unique_ptr<type::Type> m_from;
        std::unique_ptr<type::Type> m_to;
    };
}
}
