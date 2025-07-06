#pragma once

#include <cstdint>
#include <optional>
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
    CHAR,
    CHAR_PTR,
};

struct TempVar {
    uint32_t idx;
    OperandType type;
};

using ptr_type = uint64_t;

struct PtrVar {
    ptr_type offset;
    OperandType type;
};

struct Nil { };

using Operand = std::variant<int, double, TempVar, Nil, bool, char, PtrVar>;

std::string to_string(const Operand& operand);
std::string to_string(const OperandType& operand_type);
OperandType get_type(const Operand& operand);
std::optional<OperandType> from_str(const std::string& type_name);
Operand default_operand(OperandType type);

bool is_number(const Operand& operand);
bool is_number(const OperandType type);
bool is_ptr(const OperandType type);

}