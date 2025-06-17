#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include "Utils.hpp"

namespace jl {

struct TempVar {
    uint32_t idx;
};

struct Nil {};

using Operand = std::variant<int, double, TempVar, Nil>;

inline std::string to_string(const Operand& operand)
{
    switch (operand.index()) {
    case 0:
        return std::to_string(std::get<int>(operand));
    case 1:
        return std::to_string(std::get<double>(operand));
    case 2:
        return "T[" + std::to_string(std::get<TempVar>(operand).idx) + "]";
    case 3:
        return "Nil";
    }

    unimplemented();
    return "Unimplemented";
}

}