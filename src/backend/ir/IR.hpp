#pragma once

#include <cstdint>
namespace jl {
namespace ir {

    class IR {
    public:
        uint32_t m_line;

        IR(uint32_t line);
    };
}
}
