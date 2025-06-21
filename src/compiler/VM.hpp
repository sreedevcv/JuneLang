#pragma once

#include "Chunk.hpp"
#include <cstdint>

namespace jl {

class VM {
public:
    enum InterpretResult {
        OK,
        COMPILER_ERROR,
        RUNTIME_ERROR,
    };

    std::pair<InterpretResult, std::vector<Operand>> run(const Chunk& chunk);

private:
    uint32_t m_ip { 0 };
};

}