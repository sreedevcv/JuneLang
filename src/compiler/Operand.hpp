#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace jl {

// enum class DataType {
//     INT,
//     FLOAT,
//     BOOL,
//     NIL,
// };

enum class OperandType {
    INT,
    FLOAT,
    TEMP,
    NIL,
    BOOL,
    UNASSIGNED, // Not represend in Operand variant
};

struct TempVar {
    uint32_t idx;
};

struct Nil { };

using Operand = std::variant<int, double, TempVar, Nil, bool>;

std::string to_string(const Operand& operand);
std::string to_string(const Operand& operand, const std::vector<OperandType>& var_map);
std::string to_string(const OperandType& operand_type);
OperandType get_type(const Operand& operand);
OperandType get_nested_type(const Operand& operand, const std::vector<OperandType>& var_map);
std::optional<OperandType> infer_type_for_binary(
    const Operand& op1,
    const Operand& op2,
    const std::vector<OperandType>& var_map);
bool is_number(const Operand& operand);

// inline DataType get_data_type(const Operand& operand)
// {
//     switch (get_type(operand)) {
//     case OperandType::INT:
//         return DataType::INT;
//     case OperandType::FLOAT:
//         return DataType::INT;
//     case OperandType::TEMP:
//         return std::get<TempVar>(operand).type;
//     case OperandType::NIL:
//         return DataType::NIL;
//     case OperandType::BOOL:
//     }

//     unimplemented();
//     return DataType::UNASSIGNED;
// }

}