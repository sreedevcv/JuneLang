#include "Chunk.hpp"
#include "ErrorHandler.hpp"
#include "Ir.hpp"
#include "OpCode.hpp"
#include "Operand.hpp"
#include "Utils.hpp"

#include <cassert>
#include <cstdint>
#include <iomanip>
#include <ostream>

const std::vector<jl::Ir>& jl::Chunk::get_ir() const
{
    return m_ir;
}

uint32_t jl::Chunk::get_max_allocated_temps() const
{
    return m_temp_var_count;
}

std::string jl::Chunk::disassemble() const
{
    std::stringstream out;
    output_var_map(out);
    out << '\n';

    uint32_t line = -1;

    for (int i = 0; i < m_ir.size(); i++) {
        if (m_lines[i] != line) {
            line = m_lines[i];
            out << std::setfill('0') << std::setw(4) << line;
        } else {
            out << "  | ";
        }

        out << ' ';
        out << std::setfill('0') << std::setw(4) << i;
        out << std::setfill(' ') << std::setw(10) << jl::to_string(m_ir[i].dest());
        out << " : ";
        out << std::setfill(' ') << std::setw(10) << jl::to_string(m_ir[i].opcode());
        switch (m_ir[i].type()) {
        case Ir::BINARY:
            out << std::setfill(' ') << std::setw(10) << jl::to_string(m_ir[i].binary().op1);
            out << std::setfill(' ') << std::setw(10) << jl::to_string(m_ir[i].binary().op2);
            break;
        case Ir::UNARY:
            out << std::setfill(' ') << std::setw(10) << jl::to_string(m_ir[i].unary().operand);
            break;
        default:
            unimplemented();
        }
        out << '\n';
    }

    return out.str();
}

void jl::Chunk::output_var_map(std::ostream& in) const
{
    for (const auto& [name, var] : m_variable_map) {
        in << std::setfill(' ') << std::setw(12) << name;
        in << " : ";
        in << std::setfill(' ') << std::setw(3) << jl::to_string(var);
        in << '\n';
    }
}

jl::TempVar jl::Chunk::write(
    OpCode opcode,
    Operand op1,
    Operand op2,
    uint32_t line)
{
    TempVar dest = create_temp_var();

    m_ir.push_back(Ir { BinaryIr {
        opcode,
        op1,
        op2,
        dest,
    } });

    m_lines.push_back(line);

    return dest;
}

void jl::Chunk::write_with_dest(
    OpCode opcode,
    Operand op1,
    Operand op2,
    TempVar dest,
    uint32_t line)
{
    m_ir.push_back(Ir { BinaryIr {
        opcode,
        op1,
        op2,
        dest,
    } });

    m_lines.push_back(line);
}

jl::TempVar jl::Chunk::write(
    OpCode opcode,
    Operand operand,
    uint32_t line)
{
    TempVar dest = create_temp_var();

    m_ir.push_back(Ir { UnaryIr {
        opcode,
        operand,
        dest,
    } });

    m_lines.push_back(line);

    return dest;
}

void jl::Chunk::write_with_dest(
    OpCode opcode,
    Operand operand,
    TempVar dest,
    uint32_t line)
{
    m_ir.push_back(Ir { UnaryIr {
        opcode,
        operand,
        dest,
    } });

    m_lines.push_back(line);
}

jl::TempVar jl::Chunk::store_variable(const std::string& var_name)
{
    auto temp = std::string("temp");
    if (m_variable_map.contains(var_name)) {
        ErrorHandler::error(temp, 1, "Redeclaration of a variable");
    }

    TempVar var = create_temp_var();
    m_variable_map.insert(std::pair { var_name, var });

    return var;
}

std::optional<jl::TempVar> jl::Chunk::look_up_variable(const std::string& var_name) const
{
    return m_variable_map.contains(var_name)
        ? std::optional { m_variable_map.at(var_name) }
        : std::nullopt;
}

jl::TempVar jl::Chunk::create_temp_var()
{
    return TempVar {
        .idx = m_temp_var_count++
    };
}

const std::unordered_map<std::string, jl::TempVar>& jl::Chunk::get_variable_map() const
{
    return m_variable_map;
}