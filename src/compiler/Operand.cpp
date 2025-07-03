#include "Operand.hpp"

#include "Utils.hpp"
#include <optional>
#include <string>

// static std::string to_string(const jl::TempVar& var)
// {
//     switch (var.type) {
//     case jl::OperandType::INT:
//         return "I[" + std::to_string(var.idx) + "]";
//     case jl::OperandType::FLOAT:
//         return "F[" + std::to_string(var.idx) + "]";
//     case jl::OperandType::TEMP:
//         // unimplemented();
//         return "T[" + std::to_string(var.idx) + "]";
//     case jl::OperandType::NIL:
//         return "N[" + std::to_string(var.idx) + "]";
//     case jl::OperandType::BOOL:
//         return "B[" + std::to_string(var.idx) + "]";
//     case jl::OperandType::UNASSIGNED:
//         return "U[" + std::to_string(var.idx) + "]";
//         break;
//     }
// }

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
        break;
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
        return OperandType::UNASSIGNED;
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
    default:
        unimplemented();
        break;
    }

    return Nil {};
}