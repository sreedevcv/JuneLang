#pragma once

#include <cstdint>
#include <string>
#include <variant>

namespace jl {
struct LiteralValue {
    using int_type = int64_t;
    using float_type = double;
    using bool_type = bool;
    using char_type = char;
    using type = std::variant<int_type, float_type, bool_type, char_type>;

    type data;

    LiteralValue(const type& data);

    std::string to_str() const;

    bool operator==(const LiteralValue& other) const;

};
};
