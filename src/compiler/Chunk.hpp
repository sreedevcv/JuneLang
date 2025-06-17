#pragma once

#include <cstdint>
#include <vector>

#include "OpCode.hpp"
#include "Operand.hpp"
#include "Value.hpp"

namespace jl {

struct Ir {
    OpCode opcode;
    Operand op1;
    Operand op2;
    TempVar dest;
};

class Chunk {
public:
    uint8_t add_constant(Value&& constant);
    void add_opcode(OpCode opcode, uint32_t line);
    std::string disassemble();

    template <typename T>
    T read_byte(uint32_t idx) const
    {
        return static_cast<T>(m_opcodes[idx]);
    }

    jl::Value read_constant(uint8_t offset) const;

    TempVar write(OpCode opcode,
        Operand op1,
        Operand op2);

private:
    std::vector<Ir> m_ops;
    uint32_t m_temp_var_count { 0 };

    //////////////////////////////////////////////////////////
    std::vector<OpCode> m_opcodes;
    std::vector<uint32_t> m_lines;
    std::vector<Value> m_constants;
    
    // uint32_t write_simple_instruction(std::ostream& out, OpCode opcode, uint32_t offset) const;
    // uint32_t write_constant_instruction(std::ostream& out, OpCode opcode, uint32_t offset) const;
    // uint32_t disasemble_opcode(std::ostream& out, uint32_t offset) const;
};

}