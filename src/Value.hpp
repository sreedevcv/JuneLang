#pragma once

#include "Ref.hpp"
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

class JlValue : public Ref {
public:
    using Value
        = std::variant<
            int,
            double,
            bool,
            std::string,
            JNullType, // Respresents null value
            Callable*,
            Instance*,
            std::vector<Expr*>*>;

    Value value;

    JlValue() = default;
    JlValue(const Value& value);
    JlValue(const Value&& value);
    size_t index() const;
    Value& get();
};

} // namespace jl
