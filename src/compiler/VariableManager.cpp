#include "VariableManager.hpp"
#include "OpCode.hpp"
#include "Operand.hpp"
#include "Utils.hpp"
#include <cstdint>

jl::TempVar jl::VariableManager::create_temp_var(OperandType type)
{
    const auto var = TempVar {
        m_var_count++,
    };

    m_var_types.push_back(type);

    return var;
}

jl::TempVar jl::VariableManager::store_variable(const std::string& var_name, OperandType type)
{
    // TODO:: Return nullopt and handle redeclaration error
    if (m_variable_map.contains(var_name)) {
        // ErrorHandler::error(
        //     m_file_name,
        //     1,
        //     std::format("Redeclaration of a variable: {}", var_name)
        //         .c_str());

        // For now we replace the existing varible with this one
    }

    TempVar var = create_temp_var(type);
    m_variable_map.insert(std::pair { var_name, var.idx });

    return var;
}

std::optional<jl::TempVar> jl::VariableManager::look_up_variable(const std::string& var_name) const
{
    if (m_variable_map.contains(var_name)) {
        return TempVar { m_variable_map.at(var_name) };

    } else {
        return std::nullopt;
    }
}

uint32_t jl::VariableManager::get_max_allocated_temps() const
{
    return m_var_count;
}

jl::OperandType jl::VariableManager::get_nested_type(const Operand& operand) const
{
    return get_type(operand) == OperandType::TEMP
        ? m_var_types[std::get<TempVar>(operand).idx]
        : get_type(operand);
}

const std::string& jl::VariableManager::get_variable_name_from_temp_var(uint32_t idx) const
{
    for (const auto& [name, var] : m_variable_map) {
        if (var == idx) {
            return name;
        }
    }

    return get_variable_map().begin()->first;
}

std::optional<jl::OperandType> jl::VariableManager::infer_type_for_binary(
    const Operand& op1,
    const Operand& op2,
    OpCode opcode) const
{
    const auto t1 = get_nested_type(op1);
    const auto t2 = get_nested_type(op2);
    const auto operator_type = get_category(opcode);

    switch (operator_type) {
    case OperatorCategory::ARITHAMETIC:
        if (is_number(t1) && is_number(t2)) {
            if (t1 == OperandType::FLOAT || t2 == OperandType::FLOAT) {
                return OperandType::FLOAT;
            } else {
                return OperandType::INT;
            }
        } else if (is_ptr(t1) && is_ptr(t2)) {
            // Pointer arithmetics
            if (t1 == OperandType::INT) {
                return t2;
            } else if (t2 == OperandType::INT) {
                return t1;
            } else if (t1 == t2) {
                return t1;
            }
        }
        break;
    case OperatorCategory::COMPARISON:
        if (is_number(t1) && is_number(t2)) {
            return OperandType::BOOL;
        } else if (t1 == t2) {
            return OperandType::BOOL;
        }
        break;
    case OperatorCategory::BOOLEAN:
        if (t1 == OperandType::BOOL && t2 == OperandType::BOOL) {
            return OperandType::BOOL;
        }
        break;
    case OperatorCategory::OTHER:
        break;
    }

    return std::nullopt;
}

const std::unordered_map<std::string, uint32_t>& jl::VariableManager::get_variable_map() const
{
    return m_variable_map;
}

void jl::VariableManager::set_var_data_type(uint32_t idx, OperandType type)
{
    m_var_types[idx] = type;
}

jl::OperandType jl::VariableManager::get_var_data_type(uint32_t idx) const
{
    return m_var_types[idx];
}

std::string jl::VariableManager::pretty_print(const Operand& operand) const
{
    switch (get_type(operand)) {
    case OperandType::TEMP: {
        const auto var = std::get<TempVar>(operand).idx;
        switch (m_var_types[var]) {
        case jl::OperandType::INT:
            return "I[" + std::to_string(var) + "]";
        case jl::OperandType::FLOAT:
            return "F[" + std::to_string(var) + "]";
        case jl::OperandType::TEMP:
            // unimplemented();
            return "T[" + std::to_string(var) + "]";
        case jl::OperandType::NIL:
            return "N[" + std::to_string(var) + "]";
        case jl::OperandType::BOOL:
            return "B[" + std::to_string(var) + "]";
        case jl::OperandType::UNASSIGNED:
            return "U[" + std::to_string(var) + "]";
        case OperandType::CHAR:
            return "C[" + std::to_string(var) + "]";
        case OperandType::CHAR_PTR:
            return "CP[" + std::to_string(var) + "]";
        case OperandType::INT_PTR:
            return "IP[" + std::to_string(var) + "]";
        case OperandType::FLOAT_PTR:
            return "FP[" + std::to_string(var) + "]";
        case OperandType::BOOL_PTR:
            return "BP[" + std::to_string(var) + "]";
        case OperandType::PTR:
            return "PP[" + std::to_string(var) + "]";
        }
    } break;
    case OperandType::INT:
    case OperandType::FLOAT:
    case OperandType::NIL:
    case OperandType::BOOL:
    case OperandType::UNASSIGNED:
    case OperandType::CHAR:
    case OperandType::CHAR_PTR:
    case OperandType::INT_PTR:
    case OperandType::FLOAT_PTR:
    case OperandType::BOOL_PTR:
    case OperandType::PTR:
        return to_string(operand);
        break;
    }

    unimplemented();
    return "Unimplemented";
}