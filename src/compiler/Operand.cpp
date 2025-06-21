#include "Operand.hpp"

#include "Utils.hpp"
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

std::string jl::to_string(const Operand& operand, const std::vector<OperandType>& var_map)
{
    switch (get_type(operand)) {
    case OperandType::TEMP: {
        const auto var = std::get<TempVar>(operand).idx;
        switch (var_map[var]) {
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
            break;
        }
    } break;
    case OperandType::INT:
    case OperandType::FLOAT:
    case OperandType::NIL:
    case OperandType::BOOL:
    case OperandType::UNASSIGNED:
        return to_string(operand);
    }

    unimplemented();
    return "Unimplemented";
}

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

jl::OperandType jl::get_nested_type(const Operand& operand, const std::vector<OperandType>& var_map)
{
    return get_type(operand) == OperandType::TEMP
        ? var_map[std::get<TempVar>(operand).idx]
        : get_type(operand);
}

std::optional<jl::OperandType> jl::infer_type_for_binary(
    const Operand& op1,
    const Operand& op2,
    const std::vector<OperandType>& var_map)
{
    const auto t1 = get_nested_type(op1, var_map);
    const auto t2 = get_nested_type(op2, var_map);

    if (t1 == t2) {
        return t1;
    } else if (is_number(op1) && is_number(op2)) {
        if (t1 == OperandType::FLOAT || t2 == OperandType::FLOAT) {
            return OperandType::FLOAT;
        } else {
            return OperandType::INT;
        }
    } else {
        return std::nullopt;
    }
}

// const auto inferred_type = infer_type(op1, op2);