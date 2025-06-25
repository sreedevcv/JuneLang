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
#include <ostream>

jl::Chunk::Chunk(std::string name)
    : m_name(std::move(name))
{
}

const std::vector<jl::Ir>& jl::Chunk::get_ir() const
{
    return m_ir;
}

uint32_t jl::Chunk::get_max_allocated_temps() const
{
    return m_var_manager.get_max_allocated_temps();
}

std::string jl::Chunk::disassemble() const
{
    std::stringstream out;

    out << "------------[" << m_name << "]------------\n";

    out << "OUTPUT: ";
    out << to_string(return_type) << "\t";
    out << "INPUTS: ";
    for (int i = 0; i < m_inputs.size(); i++) {
        out << "<" << m_inputs[i] << "> ";
    }
    out << "\n";

    output_var_map(out);
    out << '\n';

    uint32_t line = -1;

    for (int i = 0; i < m_ir.size(); i++) {
        // Print line number
        if (m_lines[i] != line) {
            line = m_lines[i];
            out << std::setfill('0') << std::setw(4) << line;
        } else {
            out << "  | ";
        }

        // Print ir index
        out << ' ';
        out << std::setfill('0') << std::setw(4) << i;

        // Print destination and opcode
        if (m_ir[i].type() == Ir::BINARY || m_ir[i].type() == Ir::UNARY) {
            out << std::setfill(' ') << std::setw(10) << m_var_manager.pretty_print(m_ir[i].dest());
            out << " : ";
            out << std::setfill(' ') << std::setw(10) << jl::to_string(m_ir[i].opcode());
        } else {
            out << std::setfill(' ') << std::setw(10) << ' ';
            out << "   ";
            out << std::setfill(' ') << std::setw(10) << jl::to_string(m_ir[i].opcode());
        }

        switch (m_ir[i].type()) {
        case Ir::BINARY:
            out << std::setfill(' ') << std::setw(10) << m_var_manager.pretty_print(m_ir[i].binary().op1);
            out << std::setfill(' ') << std::setw(10) << m_var_manager.pretty_print(m_ir[i].binary().op2);
            break;
        case Ir::UNARY:
            out << std::setfill(' ') << std::setw(10) << m_var_manager.pretty_print(m_ir[i].unary().operand);
            break;
        case Ir::CONTROL:
            out << std::setfill(' ') << std::setw(10) << "{" << to_string(m_ir[i].control().data) << "}";
            break;
        case Ir::JUMP:
            out << std::setfill(' ') << std::setw(10) << "{" << std::get<int>(m_ir[i].jump().label) << "}";
            out << std::setfill(' ') << std::setw(10) << m_var_manager.pretty_print(m_ir[i].jump().data);
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
    for (const auto& [name, var] : m_var_manager.get_variable_map()) {
        in << std::setfill(' ') << std::setw(12) << name;
        in << " : ";
        in << std::setfill(' ') << std::setw(3) << std::to_string(var);
        in << '\n';
    }
}

jl::OperandType jl::Chunk::handle_binary_type_inference(jl::Operand op1, jl::Operand op2, OpCode opcode, uint32_t line)
{
    const auto inferred_type = m_var_manager.infer_type_for_binary(op1, op2, opcode);

    // Handle error
    if (!inferred_type) {
        ErrorHandler::error(
            m_file_name,
            line,
            std::format(
                "[Druing codegen] Left[{}] and right[{}] types do not match",
                m_var_manager.pretty_print(op1),
                m_var_manager.pretty_print(op2))
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
    const auto inferred_type = handle_binary_type_inference(op1, op2, opcode, line);
    const auto dest = m_var_manager.create_temp_var(inferred_type);

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
    const auto inferred_type = handle_binary_type_inference(op1, op2, opcode, line);
    // Get the destination type from look up table
    const auto dest_type = m_var_manager.get_var_data_type(dest.idx);

    // If its a new variable
    if (dest_type == OperandType::UNASSIGNED) {
        m_var_manager.set_var_data_type(dest.idx, inferred_type); // Update the look up table
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
    const auto type = m_var_manager.get_nested_type(operand);

    if (type == OperandType::UNASSIGNED) {
        ErrorHandler::error(m_file_name, line, "Use of unintialized variable");
        return m_var_manager.create_temp_var(type);
    }

    TempVar dest = m_var_manager.create_temp_var(type);

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
    const auto type = m_var_manager.get_nested_type(operand);

    if (type == OperandType::UNASSIGNED) {
        ErrorHandler::error(m_file_name, line, "Use of unintialized variable");
        return;
    } else {
        const auto dest_type = m_var_manager.get_var_data_type(dest.idx);

        // Update type data
        if (dest_type == OperandType::UNASSIGNED) {
            m_var_manager.set_var_data_type(dest.idx, type);
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

void jl::Chunk::write_control(OpCode opcode, Operand data, uint32_t line)
{
    if (opcode == OpCode::LABEL && get_type(data) != OperandType::INT) {
        ErrorHandler::error(m_file_name, line, "Label needs an int");
        return;
    }

    m_ir.push_back(Ir {
        ControlIr {
            .opcode = opcode,
            .data = data } });

    m_lines.push_back(line);
}

void jl::Chunk::write_jump(OpCode opcode, Operand data, Operand label, uint32_t line)
{
    m_ir.push_back(Ir {
        JumpIr {
            .opcode = opcode,
            .data = data,
            .label = label } });

    m_lines.push_back(line);
}

const std::unordered_map<std::string, uint32_t>& jl::Chunk::get_variable_map() const
{
    return m_var_manager.get_variable_map();
}

jl::TempVar jl::Chunk::store_variable(const std::string& var_name, OperandType type)
{
    return m_var_manager.store_variable(var_name, type);
}

std::optional<jl::TempVar> jl::Chunk::look_up_variable(const std::string& var_name) const
{
    return m_var_manager.look_up_variable(var_name);
}

int32_t jl::Chunk::create_new_label()
{
    return m_label_count++;
}

uint32_t jl::Chunk::get_last_line() const
{
    return m_lines.empty() ? 0 : m_lines.back();
}

int32_t jl::Chunk::get_max_labels() const
{
    return m_label_count;
}

jl::OperandType jl::Chunk::get_nested_type(const Operand& operand) const
{
    return m_var_manager.get_nested_type(operand);
}

jl::TempVar jl::Chunk::add_input_parameter(const std::string& name, OperandType type)
{
    m_inputs.push_back(name);
    return store_variable(name, type);
}

const std::vector<std::string>& jl::Chunk::get_input_variable_names() const
{
    return m_inputs;
}