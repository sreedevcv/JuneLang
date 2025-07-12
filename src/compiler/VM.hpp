#pragma once

#include <cstdint>
#include <map>
#include <stack>
#include <vector>

#include "CFFI.hpp"
#include "Chunk.hpp"
#include "Operand.hpp"

namespace jl {

class VM {
public:
    enum InterpretResult {
        OK,
        COMPILER_ERROR,
        RUNTIME_ERROR,
    };

    VM(std::map<std::string, Chunk>& chunk_map, ptr_type data_address);

    std::pair<InterpretResult, std::vector<Operand>> run();

    std::pair<InterpretResult, std::vector<Operand>> interactive_execute();

private:
    std::map<std::string, Chunk>& m_chunk_map;
    ptr_type m_base_address;
    std::stack<Operand> m_stack;
    bool debug_run = false;

    CFFI m_ffi { "/lib64/libc.so.6" };

    InterpretResult run(
        Chunk& chunk,
        std::vector<Operand>& temp_vars);

    uint32_t execute_ir(
        Ir ir,
        uint32_t pc,
        Chunk& chunk,
        std::vector<Operand>& temp_vars,
        const std::vector<uint32_t> locations);

    void handle_binary_ir(const Ir& ir, std::vector<Operand>& temp_vars);

    void handle_unary_ir(
        const Ir& ir,
        std::vector<Operand>& temp_vars);

    uint32_t handle_control_ir(
        const uint32_t pc,
        const Ir& ir,
        std::vector<Operand>& temp_vars,
        const std::vector<uint32_t>& label_locations);

    void handle_type_cast(const Ir& ir, std::vector<Operand>& temp_vars);

    std::vector<uint32_t> fill_labels(const std::vector<Ir>& irs, uint32_t max_labels) const;

    void debug_print(
        const Chunk& chunk,
        uint32_t pc,
        const Ir& ir,
        const std::vector<Operand>& temp_vars);

    Operand run_function(
        const CallIr& ir,
        Chunk& func_chunk,
        const std::vector<Operand>& temp_vars);

    void patch_memmory_address(std::map<std::string, Chunk>& chunk_map, uint64_t base_address);

    template <typename T>
    static T read_data(ptr_type offset)
    {
        T data = *(T*)(offset);
        return data;
    }

    template <typename T>
    static void set_data(ptr_type offset, T& data)
    {
        *(T*)(offset) = data;
    }
};

}