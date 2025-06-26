#pragma once

#include "OpCode.hpp"
#include "Operand.hpp"

#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vector>

namespace jl {

class VariableManager {
public:
    uint32_t get_max_allocated_temps() const;
    TempVar create_temp_var(OperandType type);
    OperandType get_var_data_type(uint32_t idx) const;
    TempVar store_variable(const std::string& var_name, OperandType type);
    void set_var_data_type(uint32_t idx, OperandType type);
    std::string pretty_print(const Operand& operand) const;
    OperandType get_nested_type(const Operand& operand) const;
    const std::unordered_map<std::string, uint32_t>& get_variable_map() const;
    std::optional<TempVar> look_up_variable(const std::string& var_name) const;
    std::optional<OperandType> infer_type_for_binary(
        const Operand& op1,
        const Operand& op2,
        OpCode opcode) const;
    const std::string& get_variable_name_from_temp_var(uint32_t idx) const;

private:
    static uint32_t constexpr max_data_types { 6 };
    uint32_t m_var_count { 0 };
    std::vector<OperandType> m_types;
    std::unordered_map<std::string, uint32_t> m_variable_map;
};

}