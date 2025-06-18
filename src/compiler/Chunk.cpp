#include "Chunk.hpp"
#include "ErrorHandler.hpp"
#include "OpCode.hpp"
#include "Operand.hpp"

#include <cassert>
#include <iomanip>
#include <ostream>

std::string jl::Chunk::disassemble() const
{
    std::stringstream out;

    output_var_map(out);
    out << '\n';

    for (int i = 0; i < m_ops.size(); i++) {
        out << std::setfill('0') << std::setw(4) << i;
        out << std::setfill(' ') << std::setw(10) << jl::to_string(m_ops[i].dest);
        out << " : ";
        out << std::setfill(' ') << std::setw(10) << jl::to_string(m_ops[i].opcode);
        out << std::setfill(' ') << std::setw(10) << jl::to_string(m_ops[i].op1);
        out << std::setfill(' ') << std::setw(10) << jl::to_string(m_ops[i].op2);
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

jl::TempVar jl::Chunk::write(OpCode opcode,
    Operand op1,
    Operand op2)
{
    TempVar dest = create_temp_var();

    m_ops.push_back(Ir {
        .opcode = opcode,
        .op1 = op1,
        .op2 = op2,
        .dest = dest,
    });

    return dest;
}

void jl::Chunk::write_with_dest(OpCode opcode,
    Operand op1,
    Operand op2,
    TempVar dest)
{
    m_ops.push_back(Ir {
        .opcode = opcode,
        .op1 = op1,
        .op2 = op2,
        .dest = dest,
    });
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