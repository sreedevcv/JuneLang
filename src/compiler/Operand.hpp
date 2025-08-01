#pragma once

#include "TypeInfo.hpp"
#include <cstddef>
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
    INT_PTR,
    FLOAT_PTR,
    BOOL_PTR,
    NIL_PTR
};

struct TempVar {
    uint32_t idx;
};

using ptr_type = uint64_t;
using int_type = int32_t;
using float_type = double;

struct PtrVar {
    ptr_type offset;
    OperandType type;
};

struct Nil { };

using Operand = std::variant<int_type, float_type, TempVar, Nil, bool, char, PtrVar>;

std::string to_string(const Operand& operand);
std::string to_string(const OperandType& operand_type);
OperandType get_type(const Operand& operand);
std::optional<OperandType> from_str(const std::string& type_name);
std::optional<OperandType> from_typeinfo(const TypeInfo& type_info);
Operand default_operand(OperandType type);

bool is_number(const Operand& operand);
bool is_number(const OperandType type);
bool is_ptr(const OperandType type);
bool is_pure_ptr(const OperandType type);

std::optional<OperandType> into_ptr(OperandType type);
std::optional<OperandType> from_ptr(OperandType type);

size_t size_of_type(OperandType type);

//

template <OperandType T>
struct PrimitiveType { };

template <>
struct PrimitiveType<OperandType::INT> {
    using type = int_type;
};

template <>
struct PrimitiveType<OperandType::FLOAT> {
    using type = float_type;
};

template <>
struct PrimitiveType<OperandType::BOOL> {
    using type = bool;
};

template <>
struct PrimitiveType<OperandType::CHAR> {
    using type = char;
};

template <>
struct PrimitiveType<OperandType::NIL_PTR> {
    using type = ptr_type;
};

template <>
struct PrimitiveType<OperandType::BOOL_PTR> {
    using type = ptr_type;
};

template <>
struct PrimitiveType<OperandType::CHAR_PTR> {
    using type = ptr_type;
};

template <>
struct PrimitiveType<OperandType::INT_PTR> {
    using type = ptr_type;
};

template <>
struct PrimitiveType<OperandType::FLOAT_PTR> {
    using type = ptr_type;
};

}