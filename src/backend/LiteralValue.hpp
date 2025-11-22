#pragma once

#include <cstdint>
#include <variant>

namespace jl {
class LiteralValue {
public:
    using int_type = int64_t;
    using float_type = double;
    using type = std::variant<int_type, float_type>;

    LiteralValue(const type& data);

private:
    type m_data;
};
};
