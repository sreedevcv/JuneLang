#pragma once

#include "Value.hpp"
#include <cstdint>
#include <string>

namespace jl {
namespace value {
    class Variable : Value {
    public:
        Variable(uint32_t id);

        std::string to_str() const;

    private:
        uint32_t m_id;
    };
}
}
