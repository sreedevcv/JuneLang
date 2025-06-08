#include "Chunk.hpp"
#include "OpCode.hpp"
#include "Value.hpp"

#include <cassert>
#include <cstdint>
#include <iomanip>

uint8_t jl::Chunk::add_constant(Value&& constant)
{
    assert((m_constants.size() < 256) && "Does not support more than 256 constants in a chunk");
    m_constants.emplace_back(std::move(constant));
    return m_constants.size() - 1;
}

void jl::Chunk::add_opcode(OpCode opcode, uint32_t line)
{
    m_opcodes.push_back(opcode);
    m_lines.push_back(line);
}

std::string jl::Chunk::disassemble()
{
    std::stringstream out;

    uint32_t offset = 0;

    while (offset < m_opcodes.size()) {
        offset = disasemble_opcode(out, offset);
    }

    return out.str();
}

uint32_t jl::Chunk::disasemble_opcode(std::stringstream& out, uint32_t offset)
{
    const auto byte = m_opcodes[offset];
    const auto line = m_lines[offset];
    const char* code = jl::to_string(byte);

    // Print offset within chunk
    out << std::setw(4) << offset;
    out << '\t';

    // Print line
    if (offset > 0 && m_lines[offset] == m_lines[offset - 1]) {
        out << std::setw(4) << '|';
    } else {
        out << std::setw(4) << m_lines[offset];
    }

    // Print opcode
    switch (byte) {
    case OpCode::RETURN:
        offset = write_simple_instruction(out, byte, offset);
        break;
    case OpCode::CONSTANT:
        offset = write_constant_instruction(out, byte, offset);
        break;
    }

    // Newline
    out << '\n';

    return offset;
}

uint32_t jl::Chunk::write_simple_instruction(std::stringstream& out, OpCode opcode, uint32_t offset)
{
    out << std::setw(10) << jl::to_string(opcode);
    return offset + 1;
}

uint32_t jl::Chunk::write_constant_instruction(std::stringstream& out, OpCode opcode, uint32_t offset)
{
    auto index_to_constants = static_cast<uint32_t>(m_opcodes[offset + 1]);
    // Print "CONSTANT"
    out << std::setw(10) << jl::to_string(opcode);
    // Print index to m_constants
    out << '[' << std::setw(4) << index_to_constants << ']';
    // Print actual constant
    out << ' ' << jl::stringify(&m_constants[index_to_constants]);

    return offset + 2;
}