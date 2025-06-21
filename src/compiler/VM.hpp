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

    void handle_binary_ir(const Ir& ir, std::vector<Operand>& temp_vars);
    void handle_unary_ir(const Ir& ir, std::vector<Operand>& temp_vars);
};

}