#pragma once

#include "ir/IRVisitor.hpp"
#include <cstdint>
#include <string>

namespace jl {
namespace ir {
    struct IR {
        IR(uint32_t line);

        virtual ~IR() = default;

        virtual std::string to_str() const = 0;

        virtual void accept(IRVisitor& visitor) = 0;

        uint32_t line() const;

        uint32_t m_line;

        IR* prev = nullptr;

        IR* next = nullptr;
    };
}
}
