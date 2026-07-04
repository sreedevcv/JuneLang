#pragma once

#include "ir/IRVisitor.hpp"
#include "value/Variable.hpp"
#include <cstdint>
#include <string>

namespace jl {
class BasicBlock;
namespace ir {
    struct IR {
        IR(uint32_t line);

        virtual ~IR() = default;

        virtual std::string to_str() const = 0;

        virtual void accept(IRVisitor& visitor) = 0;

        virtual bool uses(value::Variable variable) = 0;

        virtual void replace(value::Variable from, value::Variable to) = 0;

        virtual std::optional<value::Variable> def() = 0;

        uint32_t line() const;

        uint32_t m_line;

        IR* prev = nullptr;

        IR* next = nullptr;

        BasicBlock* parent = nullptr;

        // Utilities
        void insert_before(IR* ir);

        void remove();

        void replace(IR* ir);
    };
}
}
