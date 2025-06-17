#include "Chunk.hpp"
#include "OpCode.hpp"
#include "Value.hpp"

#include <cassert>
#include <cstdint>
#include <iomanip>
#include <ostream>

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

    for (int i = 0; i < m_ops.size(); i++) {
        out << std::setfill('0') << std::setw(4) << i;
        out << std::setfill(' ') << std::setw(10) << jl::to_string(m_ops[i].dest);
        out << " = ";
        out << std::setfill(' ') << std::setw(10) << jl::to_string(m_ops[i].opcode);
        out << std::setfill(' ') << std::setw(10) << jl::to_string(m_ops[i].op1);
        out << std::setfill(' ') << std::setw(10) << jl::to_string(m_ops[i].op2);
        out << '\n';
    }

    return out.str();
}

// uint32_t jl::Chunk::disasemble_opcode(std::ostream& out, uint32_t offset) const
// {
//     const auto byte = m_opcodes[offset];
//     const auto line = m_lines[offset];
//     const char* code = jl::to_string(byte);

//     // Print offset within chunk
//     out << std::setfill('0') << std::setw(4) << offset;
//     out << '\t';

//     // Print line
//     if (offset > 0 && m_lines[offset] == m_lines[offset - 1]) {
//         out << std::setw(4) << '|';
//     } else {
//         out << std::setfill('0') << std::setw(4) << m_lines[offset];
//     }
//     out << '\t';

//     // Print opcode
//     switch (byte) {
//     case OpCode::RETURN:
//         offset = write_simple_instruction(out, byte, offset);
//         break;
//     case OpCode::CONSTANT:
//         offset = write_constant_instruction(out, byte, offset);
//         break;
//     case OpCode::NEGATE:
//         offset = write_simple_instruction(out, byte, offset);
//         break;
//     }

//     // Newline
//     out << '\n';

//     return offset;
// }


jl::TempVar jl::Chunk::write(OpCode opcode,
    Operand op1,
    Operand op2)
{
    TempVar dest = {
        .idx = m_temp_var_count++
    };

    m_ops.push_back(Ir {
        .opcode = opcode,
        .op1 = op1,
        .op2 = op2,
        .dest = dest,
    });

    return dest;
}