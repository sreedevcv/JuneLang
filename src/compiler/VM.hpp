#pragma once

#include "Chunk.hpp"
#include <cstdint>
#include <vector>

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
    uint32_t handle_control_ir(const uint32_t ip, const Ir& ir, std::vector<Operand>& temp_vars, const std::vector<uint32_t>& label_locations);
    std::vector<uint32_t> fill_labels(const std::vector<Ir>& irs, uint32_t max_labels) const;
};

}