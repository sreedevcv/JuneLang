#pragma once

#include <cstdint>
#include <cstring>
#include <map>
#include <stack>
#include <string>
#include <vector>

#include "CFFI.hpp"
#include "Chunk.hpp"
#include "Operand.hpp"
#include "Utils.hpp"

namespace jl {

class VM {
public:
    enum InterpretResult {
        OK,
        COMPILER_ERROR,
        RUNTIME_ERROR,
    };

    VM(const std::map<std::string, Chunk>& chunk_map, ptr_type data_address);

    std::pair<InterpretResult, std::vector<reg_type>> run();

    std::pair<InterpretResult, std::vector<reg_type>> interactive_execute();

    template <typename T>
    static T get(const ptr_type& val)
    {
        T data;
        std::memcpy(&data, &val, sizeof(T));
        // return (T) * (const T*)(&val);
        return data;
    }

    inline static std::string pretty_print(const reg_type& val, OperandType type)
    {
        switch (type) {
        case OperandType::INT:
            return std::to_string(get<int_type>(val));
        case OperandType::FLOAT:
            return std::to_string(get<float_type>(val));
        case OperandType::BOOL:
            return std::to_string(get<bool>(val));
        case OperandType::CHAR:
            return std::to_string(get<char>(val));
        case OperandType::CHAR_PTR:
        case OperandType::INT_PTR:
        case OperandType::FLOAT_PTR:
        case OperandType::BOOL_PTR:
        case OperandType::NIL_PTR:
            return std::to_string(get<reg_type>(val));
        case jl::OperandType::NIL:
            return "nil";
        case jl::OperandType::TEMP:
            return "temp";
        default:
            unimplemented();
            return "UNKNOWN";
        }
    }

private:
    const std::map<std::string, Chunk>& m_chunk_map;
    ptr_type m_base_address;
    std::stack<reg_type> m_stack;
    bool debug_run = false;
    CFFI m_ffi { "/lib64/libc.so.6" };

    using casting_func_t = void (*)(jl::reg_type&, jl::reg_type&);
    std::map<std::pair<jl::OperandType, jl::OperandType>, casting_func_t> m_dispatch_table;

    InterpretResult run(
        const Chunk& chunk,
        std::vector<reg_type>& temp_vars);

    uint32_t execute_ir(
        Ir ir,
        uint32_t pc,
        const Chunk& chunk,
        std::vector<reg_type>& temp_vars,
        const std::vector<uint32_t> locations);

    void handle_binary_ir(const Ir& ir, std::vector<reg_type>& temp_vars);

    void handle_unary_ir(
        const Ir& ir,
        std::vector<reg_type>& temp_vars);

    uint32_t handle_control_ir(
        const uint32_t pc,
        const Ir& ir,
        std::vector<reg_type>& temp_vars,
        const std::vector<uint32_t>& label_locations);

    void handle_type_cast(const Ir& ir, std::vector<reg_type>& temp_vars);

    void handle_load_store(const Ir& ir, std::vector<reg_type>& temp_vars);

    std::vector<uint32_t> fill_labels(const std::vector<Ir>& irs, uint32_t max_labels) const;

    void debug_print(
        const Chunk& chunk,
        uint32_t pc,
        const Ir& ir,
        const std::vector<reg_type>& temp_vars);

    reg_type run_function(
        const CallIr& ir,
        const Chunk& curr_chunk,
        const Chunk& func_chunk,
        const std::vector<reg_type>& temp_vars);

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