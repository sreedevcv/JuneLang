#pragma once

#include <string>
#include <variant>

// #include "Wrapper.hpp"

namespace jl {
class Instance;

using Value = std::variant<
    int,
    double,
    bool,
    std::string,
    char,
    void*,   // Callable
    Instance*
    >;
} // namespace jl
