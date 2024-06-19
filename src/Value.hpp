#pragma once

#include <string>
#include <variant>

// #include "Wrapper.hpp"

namespace jl {
class Instance;
class Callable;

using Value = std::variant<
    int,
    double,
    bool,
    std::string,
    char,
    Callable*,
    Instance*
    >;

} // namespace jl
