#pragma once

#include <string>
#include <variant>
#include <vector>

// #include "Wrapper.hpp"

namespace jl {
class Instance;
class Callable;
class Expr;

class JNullType {
public:
    inline bool operator==(const JNullType other) const
    {
        return true;
    }
};


using Value = std::variant<
    int,
    double,
    bool,
    std::string,
    JNullType,       // Respresents null value
    Callable*,
    Instance*,
    std::vector<Expr*>*
    >;

} // namespace jl
