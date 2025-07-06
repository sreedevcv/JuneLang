#pragma once

#include "Chunk.hpp"
#include "DataSection.hpp"
#include "Operand.hpp"
#include <cstdint>
#include <map>
#include <stack>
#include <vector>

namespace jl {

class VM {
public:
    enum InterpretResult {
        OK,
        COMPILER_ERROR,
        RUNTIME_ERROR,
    };

    std::pair<InterpretResult, std::vector<Operand>> run(
        const Chunk& chunk,
        const std::map<std::string, Chunk>& chunk_map,
        DataSection& data_section);

    std::pair<InterpretResult, std::vector<Operand>> interactive_execute(
        const Chunk& chunk,
        const std::map<std::string, Chunk>& chunk_map,
        DataSection& data_section);

private:
    std::stack<Operand> m_stack;

    InterpretResult run(
        const Chunk& chunk,
        const std::map<std::string, Chunk>& chunk_map,
        std::vector<Operand>& temp_vars,
        DataSection& data_section);

    uint32_t execute_ir(
        Ir ir,
        uint32_t pc,
        const Chunk& chunk,
        const std::map<std::string, Chunk>& chunk_map,
        std::vector<Operand>& temp_vars,
        const std::vector<uint32_t> locations,
        DataSection& data_section);

    void handle_binary_ir(const Ir& ir, std::vector<Operand>& temp_vars);
    void handle_unary_ir(
        const Ir& ir,
        std::vector<Operand>& temp_vars,
        DataSection& data_section);
    uint32_t handle_control_ir(
        const uint32_t pc,
        const Ir& ir,
        std::vector<Operand>& temp_vars,
        const std::vector<uint32_t>& label_locations,
        DataSection& data_section);
    std::vector<uint32_t> fill_labels(const std::vector<Ir>& irs, uint32_t max_labels) const;
    void debug_print(
        const Chunk& chunk,
        uint32_t pc,
        const Ir& ir,
        const std::vector<Operand>& temp_vars);

    bool debug_run = false;
};

}