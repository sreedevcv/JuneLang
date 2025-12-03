#pragma once

#include <cstdint>
#include <string>
#include <variant>

namespace jl {
class LiteralValue {
public:
    using int_type = int64_t;
    using float_type = double;
    using type = std::variant<int_type, float_type>;

    LiteralValue(const type& data);

    std::string to_str() const;

private:
    type m_data;
};
};
