#pragma once

#include "Value.hpp"
#include <cstdint>

namespace jl {
namespace value {
    class Variable : Value {
    public:
        Variable(uint32_t id);

    private:
        uint32_t m_id;
    };
}
}
