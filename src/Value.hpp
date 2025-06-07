#pragma once

#include "Ref.hpp"
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace jl {

class Instance;
class Callable;
class Expr;

template <typename T>
struct Markable : Ref {
    T m_value;

    template <typename... Args>
    Markable(Args&&... args)
        : m_value { std::forward<Args...>(args)... }
    {
    }

    T& get()
    {
        return m_value;
    }

    T get_copy()
    {
        return m_value;
    }
};

struct Null { };

using List = std::vector<Expr*>;

using Value = Markable<std::variant<
    int,
    double,
    bool,
    std::string,
    Callable*,
    Instance*,
    List,
    Null>>;

enum class Type {
    NONE,
    INT,
    FLOAT,
    STR,
    BOOL,
    CALL,
    OBJ,
    LIST,
    JNULL,
};

Type get_type(Value& value);

std::string stringify(Value* value);

namespace is {
    bool _int(Value& ref);
    bool _float(Value& ref);
    bool _bool(Value& ref);
    bool _str(Value& ref);
    bool _callable(Value& ref);
    bool _obj(Value& ref);
    bool _list(Value& ref);
    bool _number(Value& ref);
    bool _null(Value& ref);
    bool _same(Value& ref1, Value& ref2);
    // User needs to check they are of the same type before using
    bool _exact_same(Value& ref1, Value& ref2);
}
} // namespace jl
