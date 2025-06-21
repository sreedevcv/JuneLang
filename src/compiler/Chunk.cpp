#include "Chunk.hpp"
#include "ErrorHandler.hpp"
#include "Ir.hpp"
#include "OpCode.hpp"
#include "Operand.hpp"
#include "Utils.hpp"

#include <cassert>
#include <cstdint>
#include <format>
#include <iomanip>
#include <optional>
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
        out << std::setfill(' ') << std::setw(10) << jl::to_string(m_ir[i].dest(), m_temp_var_types);
        out << " : ";
        out << std::setfill(' ') << std::setw(10) << jl::to_string(m_ir[i].opcode());
        switch (m_ir[i].type()) {
        case Ir::BINARY:
            out << std::setfill(' ') << std::setw(10) << jl::to_string(m_ir[i].binary().op1, m_temp_var_types);
            out << std::setfill(' ') << std::setw(10) << jl::to_string(m_ir[i].binary().op2, m_temp_var_types);
            break;
        case Ir::UNARY:
            out << std::setfill(' ') << std::setw(10) << jl::to_string(m_ir[i].unary().operand, m_temp_var_types);
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
        in << std::setfill(' ') << std::setw(3) << std::to_string(var);
        in << '\n';
    }
}

jl::OperandType jl::Chunk::handle_binary_type_inference(jl::Operand op1, jl::Operand op2, uint32_t line)
{
    const auto inferred_type = infer_type_for_binary(op1, op2, m_temp_var_types);

    // Handle error
    if (!inferred_type) {
        ErrorHandler::error(
            m_file_name,
            line,
            std::format(
                "[Druing codegen] Left[{}] and right[{}] types do not match",
                to_string(op1, m_temp_var_types),
                to_string(op2, m_temp_var_types))
                .c_str(),
            line);
        // Assume that op1's type is the intended type ;)
        return get_type(op1);
    } else if (*inferred_type == OperandType::UNASSIGNED) {
        ErrorHandler::error(
            m_file_name,
            line,
            "[Druing codegen] Left and right variables are unintialized",
            line);
    }

    return *inferred_type;
}

jl::TempVar jl::Chunk::write(
    OpCode opcode,
    Operand op1,
    Operand op2,
    uint32_t line)
{
    const auto inferred_type = handle_binary_type_inference(op1, op2, line);
    const auto dest = create_temp_var(inferred_type);

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
    const auto inferred_type = handle_binary_type_inference(op1, op2, line);
    // Get the destination type from look up table
    const auto dest_type = m_temp_var_types[dest.idx];

    // If its a new variable
    if (dest_type == OperandType::UNASSIGNED) {
        m_temp_var_types[dest.idx] = inferred_type; // Update the look up table
    } else if (dest_type != inferred_type) {
        ErrorHandler::error(
            m_file_name,
            line,
            std::format(
                "[Druing codegen] Assiging uncompatible type to destination: {}",
                to_string(dest))
                .c_str(),
            line);
        return;
    }

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
    const auto type = get_nested_type(operand, m_temp_var_types);

    if (type == OperandType::UNASSIGNED) {
        ErrorHandler::error(m_file_name, line, "Use of unintialized variable");
        return create_temp_var(type);
    }

    TempVar dest = create_temp_var(type);

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
    const auto type = get_nested_type(operand, m_temp_var_types);

    if (type == OperandType::UNASSIGNED) {
        ErrorHandler::error(m_file_name, line, "Use of unintialized variable");
        return;
    } else {
        const auto dest_type = m_temp_var_types[dest.idx];

        // Update type data
        if (dest_type == OperandType::UNASSIGNED) {
            m_temp_var_types[dest.idx] = type;
        } else if (dest_type != type) {
            ErrorHandler::error(
                m_file_name,
                line,
                std::format(
                    "Operand({}) and destination({}) types don't match",
                    to_string(type),
                    to_string(dest_type))
                    .c_str());
        }
    }

    m_ir.push_back(Ir { UnaryIr {
        opcode,
        operand,
        dest,
    } });

    m_lines.push_back(line);
}

jl::TempVar jl::Chunk::store_variable(const std::string& var_name)
{
    if (m_variable_map.contains(var_name)) {
        ErrorHandler::error(
            m_file_name,
            1,
            std::format("Redeclaration of a variable: {}", var_name)
                .c_str());
        // For now we replace the existing varible with this one
    }

    TempVar var = create_temp_var(OperandType::UNASSIGNED);
    m_variable_map.insert(std::pair { var_name, var.idx });

    return var;
}

std::optional<jl::TempVar> jl::Chunk::look_up_variable(const std::string& var_name) const
{
    if (m_variable_map.contains(var_name)) {
        const auto idx = m_variable_map.at(var_name);
        return TempVar {
            .idx = idx,
        };
    } else {
        return std::nullopt;
    }
}

jl::TempVar jl::Chunk::create_temp_var(OperandType type)
{
    m_temp_var_types.push_back(type);

    return TempVar {
        .idx = m_temp_var_count++,
    };
}

const std::unordered_map<std::string, uint32_t>& jl::Chunk::get_variable_map() const
{
    return m_variable_map;
}