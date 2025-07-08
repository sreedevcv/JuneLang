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
#include <iostream>
#include <ostream>
#include <print>
#include <string>
#include <utility>

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
        print_ir(out, m_ir[i]);
        out << '\n';
    }

    return out.str();
}

std::ostream& jl::Chunk::print_ir(std::ostream& out, const Ir& ir) const
{
    if (ir.type() == Ir::BINARY || ir.type() == Ir::UNARY || ir.type() == Ir::CALL) {
        out << '\t';
        out << std::left << std::setfill(' ') << std::setw(14) << jl::to_string(ir.opcode());
    } else {
        out << '\t';
        out << std::left << std::setfill(' ') << std::setw(14) << jl::to_string(ir.opcode());
    }

    if (ir.type() == Ir::BINARY || ir.type() == Ir::UNARY || ir.type() == Ir::CALL) {
        out << std::left << std::setfill(' ') << std::setw(10) << m_var_manager.pretty_print(ir.dest());
    } else {
        out << std::left << std::setfill(' ') << std::setw(10) << ' ';
    }

    switch (ir.type()) {
    case Ir::BINARY:
        out << std::left << std::setfill(' ') << std::setw(10) << m_var_manager.pretty_print(ir.binary().op1);
        out << std::left << std::setfill(' ') << std::setw(10) << m_var_manager.pretty_print(ir.binary().op2);
        break;
    case Ir::UNARY:
        out << std::left << std::setfill(' ') << std::setw(10) << m_var_manager.pretty_print(ir.unary().operand);
        break;
    case Ir::CONTROL:
        out << std::left << std::setfill(' ') << std::setw(10) << m_var_manager.pretty_print(ir.control().data);
        break;
    case Ir::JUMP_STORE:
        if (ir.opcode() == OpCode::JMP_UNLESS)
            out << std::left << std::setfill(' ') << std::setw(10) << m_var_manager.pretty_print(ir.jump().target);
        else
            out << std::left << std::setfill(' ') << std::setw(10) << m_var_manager.pretty_print(ir.jump().target);

        out << std::left << std::setfill(' ') << std::setw(10) << m_var_manager.pretty_print(ir.jump().data);
        break;
    case Ir::CALL:
        out << '\t' << ir.call().func_name << " (";
        for (const auto& arg : ir.call().args) {
            out << (m_var_manager.pretty_print(arg)) << " ";
        }
        out << " )";
        break;
    default:
        unimplemented();
    }

    return out;
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
        std::println("{}", disassemble());

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
    TempVar op1,
    TempVar op2,
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
    TempVar op1,
    TempVar op2,
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
    } else if (type == OperandType::NIL) {
        ErrorHandler::error(m_file_name, line, "Cannot use nil type in assignemnt");
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
    } else if (type == OperandType::NIL) {
        ErrorHandler::error(m_file_name, line, "Cannot use nil type in assignemnt");
        return;
    } else {
        const auto dest_type = m_var_manager.get_var_data_type(dest.idx);

        // Update type data
        if (dest_type == OperandType::UNASSIGNED) {
            m_var_manager.set_var_data_type(dest.idx, type);
        } else if (dest_type == OperandType::NIL) {
            ErrorHandler::error(m_file_name, line, "Cannot use nil type in assignemnt");
            return;
        } else if (dest_type != type && opcode != OpCode::LOAD) {
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

void jl::Chunk::write_jump_or_store(OpCode opcode, TempVar data, Operand target, uint32_t line)
{
    m_ir.push_back(Ir {
        JumpStoreIr {
            .opcode = opcode,
            .data = data,
            .target = target } });

    m_lines.push_back(line);
}

void jl::Chunk::write_call(
    OpCode opcode,
    TempVar func_var,
    std::string func_name,
    TempVar dest,
    std::vector<TempVar>&& args,
    uint32_t line)
{
    m_ir.push_back(Ir {
        CallIr {
            .opcode = opcode,
            .func_var = func_var,
            .func_name = std::move(func_name),
            .args = std::move(args),
            .return_var = dest,
        } });

    m_lines.push_back(line);
}

const std::unordered_map<std::string, uint32_t>& jl::Chunk::get_variable_map() const
{
    return m_var_manager.get_variable_map();
}

std::optional<jl::TempVar> jl::Chunk::store_variable(const std::string& var_name, OperandType type)
{
    return m_var_manager.store_variable(var_name, type);
}

std::optional<jl::TempVar> jl::Chunk::look_up_variable(const std::string& var_name) const
{
    return m_var_manager.look_up_variable(var_name);
}

const std::string& jl::Chunk::get_variable_name_from_temp_var(uint32_t idx) const
{
    return m_var_manager.get_variable_name_from_temp_var(idx);
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
    return *store_variable(name, type);
}

jl::TempVar jl::Chunk::create_temp_var(OperandType type)
{
    return m_var_manager.create_temp_var(type);
}

const std::vector<std::string>& jl::Chunk::get_input_variable_names() const
{
    return m_inputs;
}

const std::vector<uint32_t> jl::Chunk::get_lines() const
{
    return m_lines;
}

jl::TempVar jl::Chunk::add_data(DataSection& ds, const std::string& data, OperandType type)
{
    // FIXME: Change this to avoid useless tempvar creation
    const auto offset = ds.add_data(data);
    const auto ptr = PtrVar { .offset = offset, .type = type };
    const auto ptr_var = create_temp_var(type);
    write_with_dest(OpCode::MOVE, ptr, ptr_var, get_last_line());
    return ptr_var;
}

jl::TempVar jl::Chunk::create_ptr_var(OperandType type, ptr_type offset)
{
    const auto ptr = PtrVar { .offset = offset, .type = type };
    const auto ptr_var = create_temp_var(type);
    write_with_dest(OpCode::MOVE, ptr, ptr_var, get_last_line());
    return ptr_var;
}

void jl::Chunk::push_block()
{
    m_var_manager.push_block();
}

void jl::Chunk::pop_block()
{
    m_var_manager.pop_block();
}

void jl::Chunk::register_function(uint32_t temp_var, const std::string& name)
{
    m_registered_functions.push_back({ temp_var, name });
}