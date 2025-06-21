#pragma once

#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vector>

#include "Ir.hpp"
#include "OpCode.hpp"
#include "Operand.hpp"

namespace jl {

class Chunk {
public:
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

    TempVar store_variable(const std::string& var_name);
    std::optional<TempVar> look_up_variable(const std::string& var_name) const;

    const std::vector<Ir>& get_ir() const;
    uint32_t get_max_allocated_temps() const;
    void output_var_map(std::ostream& in) const;
    const std::unordered_map<std::string, uint32_t>& get_variable_map() const;

private:
    std::string m_file_name { "test" };
    std::vector<Ir> m_ir;
    std::vector<uint32_t> m_lines;
    uint32_t m_temp_var_count { 0 };
    std::vector<OperandType> m_temp_var_types;
    std::unordered_map<std::string, uint32_t> m_variable_map;

    TempVar create_temp_var(OperandType type);
    OperandType handle_binary_type_inference(jl::Operand op1, jl::Operand op2, uint32_t line);
};

}