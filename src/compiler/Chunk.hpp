#pragma once

#include <cstdint>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "DataSection.hpp"
#include "Ir.hpp"
#include "OpCode.hpp"
#include "Operand.hpp"
#include "VariableManager.hpp"

namespace jl {

class Chunk {
public:
    Chunk(std::string name);

    std::string disassemble() const;
    TempVar write(OpCode opcode,
        Operand op1,
        Operand op2,
        uint32_t line);
    void write_with_dest(OpCode opcode,
        Operand op1,
        Operand op2,
        TempVar dest,
        uint32_t line);
    TempVar write(OpCode opcode,
        Operand operand,
        uint32_t line);
    void write_with_dest(OpCode opcode,
        Operand operand,
        TempVar dest,
        uint32_t line);
    void write_control(OpCode opcode, Operand data, uint32_t line);
    void write_jump(OpCode opcode, Operand data, Operand label, uint32_t line);
    void write_call(
        OpCode opcode,
        Operand func_var,
        std::string func_name,
        TempVar dest,
        std::vector<Operand>&& args,
        uint32_t line);

    const std::vector<Ir>& get_ir() const;
    const std::vector<uint32_t> get_lines() const;

    uint32_t get_max_allocated_temps() const;
    void output_var_map(std::ostream& in) const;
    uint32_t get_last_line() const;
    int32_t get_max_labels() const;

    TempVar store_variable(const std::string& var_name, OperandType type);
    std::optional<TempVar> look_up_variable(const std::string& var_name) const;
    const std::string& get_variable_name_from_temp_var(uint32_t idx) const;
    const std::unordered_map<std::string, uint32_t>& get_variable_map() const;
    const std::vector<std::string>& get_input_variable_names() const;

    OperandType get_nested_type(const Operand& operand) const;
    int32_t create_new_label();
    TempVar add_input_parameter(const std::string& name, OperandType type);
    TempVar create_temp_var(OperandType type);
    PtrVar add_data(DataSection& ds, const std::string& data, OperandType type);

    std::string m_name;
    OperandType return_type { OperandType::UNASSIGNED };

private:
    std::string m_file_name { "test" };
    std::vector<Ir> m_ir;
    std::vector<uint32_t> m_lines;
    VariableManager m_var_manager;
    int32_t m_label_count { 0 };
    std::vector<std::string> m_inputs;

    OperandType handle_binary_type_inference(jl::Operand op1, jl::Operand op2, OpCode opcode, uint32_t line);
};

}