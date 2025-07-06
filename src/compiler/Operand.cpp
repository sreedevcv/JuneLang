#include "Operand.hpp"

#include "Utils.hpp"
#include <optional>
#include <string>

std::string jl::to_string(const Operand& operand)
{
    switch (get_type(operand)) {
    case OperandType::INT:
        return std::to_string(std::get<int>(operand));
    case OperandType::FLOAT:
        return std::to_string(std::get<double>(operand));
    case OperandType::TEMP:
        return "[" + std::to_string(std::get<TempVar>(operand).idx) + "]";
    case OperandType::NIL:
        return "Nil";
    case OperandType::BOOL:
        return (std::get<bool>(operand) == true) ? "true" : "false";
    case OperandType::UNASSIGNED:
        return "UNKNOWN";
    case OperandType::CHAR: {
        std::string ch { '\'', std::get<char>(operand), '\'' };
        return ch;
    }
    case OperandType::CHAR_PTR:
        return "C(" + std::to_string(std::get<PtrVar>(operand).offset) + ")";
    }

    unimplemented();
    return "Unimplemented";
}

jl::OperandType jl::get_type(const Operand& operand)
{
    switch (operand.index()) {
    case 0:
        return OperandType::INT;
    case 1:
        return OperandType::FLOAT;
    case 2:
        return OperandType::TEMP;
    case 3:
        return OperandType::NIL;
    case 4:
        return OperandType::BOOL;
    case 5:
        return OperandType::CHAR;
    case 6:
        return OperandType::CHAR_PTR;
    }

    unimplemented();
    return OperandType::NIL;
}

std::string jl::to_string(const OperandType& type)
{
    switch (type) {
    case OperandType::INT:
        return "INT";
    case OperandType::FLOAT:
        return "FLOAT";
    case OperandType::TEMP:
        return "TEMP";
    case OperandType::NIL:
        return "NIL";
    case OperandType::BOOL:
        return "BOOL";
    case OperandType::UNASSIGNED:
        return "UNASSIGNED";
    case OperandType::CHAR:
        return "CHAR";
    case OperandType::CHAR_PTR:
        return "CHAR_PTR";
    }

    unimplemented();
    return "UNIMPLEMENTED";
}

bool jl::is_number(const Operand& operand)
{
    const auto type = get_type(operand);
    return type == OperandType::INT || type == OperandType::FLOAT;
}

bool jl::is_number(const OperandType type)
{
    return type == OperandType::INT || type == OperandType::FLOAT;
}

bool jl::is_ptr(const OperandType type)
{
    return type == OperandType::CHAR_PTR || type == OperandType::INT;
}

std::optional<jl::OperandType> jl::from_str(const std::string& type_name)
{
    if (type_name == "int") {
        return OperandType::INT;
    } else if (type_name == "float") {
        return OperandType::FLOAT;
    } else if (type_name == "bool") {
        return OperandType::BOOL;
    } else if (type_name == "nil") {
        return OperandType::NIL;
    } else if (type_name == "char") {
        return OperandType::CHAR;
    } else if (type_name == "char*") {
        return OperandType::CHAR;
    } else {
        return std::nullopt;
    }
}

jl::Operand jl::default_operand(OperandType type)
{
    switch (type) {
    case OperandType::INT:
        return int {};
    case OperandType::FLOAT:
        return double {};
    case OperandType::TEMP:
        return TempVar {};
    case OperandType::NIL:
        return Nil {};
    case OperandType::BOOL:
        return bool {};
    case jl::OperandType::CHAR:
        return '\0';
    case OperandType::UNASSIGNED:
        return Nil {};
    case OperandType::CHAR_PTR:
        return PtrVar { .type = OperandType::CHAR_PTR };
        break;
    }

    unimplemented();
    return Nil {};
}