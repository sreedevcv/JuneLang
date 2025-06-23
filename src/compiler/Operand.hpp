#pragma once

#include <cstdint>
#include <string>
#include <variant>

namespace jl {

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
std::string to_string(const OperandType& operand_type);
OperandType get_type(const Operand& operand);

bool is_number(const Operand& operand);
bool is_number(const OperandType type);

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