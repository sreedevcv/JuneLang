#pragma once

#include "Utils.hpp"
#include <cstdint>
#include <string>
#include <variant>

namespace jl {

struct TempVar {
    uint32_t idx;
};

struct Nil { };

using Operand = std::variant<int, double, TempVar, Nil, bool>;

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
    case 4:
        return (std::get<bool>(operand) == true) ? "true" : "false";
    }

    unimplemented();
    return "Unimplemented";
}

}