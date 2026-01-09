#pragma once

#include <cstdint>
#include <string>
#include <variant>

namespace jl {
class LiteralValue {
public:
    using int_type = int64_t;
    using float_type = double;
    using bool_type = bool;
    using char_type = char;
    using type = std::variant<int_type, float_type, bool_type, char_type>;

    LiteralValue(const type& data);

    std::string to_str() const;

    type m_data;
};
};
