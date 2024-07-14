#pragma once

#include <string>
#include <variant>
#include <vector>

// #include "Wrapper.hpp"

namespace jl {
class Instance;
class Callable;
class Expr;

using Value = std::variant<
    int,
    double,
    bool,
    std::string,
    char,
    Callable*,
    Instance*,
    std::vector<Expr*>
    >;

} // namespace jl
