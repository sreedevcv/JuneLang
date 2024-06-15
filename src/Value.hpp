#pragma once

#include <string>
#include <variant>
// #include "Callable.hpp"

namespace jl {
using Value = std::variant<
    int,
    double,
    bool,
    std::string,
    char,
    void*
    >;
} // namespace jl
