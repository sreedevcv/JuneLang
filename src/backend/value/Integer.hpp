#pragma once

#include "Value.hpp"
#include <cstdint>

namespace jl {
namespace value {
    class Integer : Value {
    public:
        using type = int64_t;

        Integer(type value);

    private:
        type m_value;
    };
}
}
