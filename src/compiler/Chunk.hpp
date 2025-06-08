#pragma once

#include <cstdint>
#include <sstream>
#include <vector>

#include "OpCode.hpp"
#include "Value.hpp"

namespace jl {

class Chunk {
public:
    uint8_t add_constant(Value&& constant);
    void add_opcode(OpCode opcode, uint32_t line);
    std::string disassemble();

private:
    std::vector<OpCode> m_opcodes;
    std::vector<uint32_t> m_lines;
    std::vector<Value> m_constants;

    uint32_t disasemble_opcode(std::stringstream& out, uint32_t offset);
    uint32_t write_simple_instruction(std::stringstream& out, OpCode opcode, uint32_t offset);
    uint32_t write_constant_instruction(std::stringstream& out, OpCode opcode, uint32_t offset);
};

}