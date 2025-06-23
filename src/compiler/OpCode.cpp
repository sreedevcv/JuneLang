#include "OpCode.hpp"
#include "Utils.hpp"

const char* jl::to_string(OpCode opcode)
{
    switch (opcode) {
    case OpCode::RETURN:
        return "RETURN";
    case OpCode::ADD:
        return "ADD";
    case OpCode::MINUS:
        return "MINUS";
    case OpCode::STAR:
        return "STAR";
    case OpCode::SLASH:
        return "SLASH";
    case OpCode::GREATER:
        return "GREATER";
    case OpCode::LESS:
        return "LESS";
    case OpCode::GREATER_EQUAL:
        return "GREATER_EQUAL";
    case OpCode::LESS_EQUAL:
        return "LESS_EQUAL";
    case OpCode::MODULUS:
        return "PERCENT";
    case OpCode::EQUAL:
        return "EQUAL_EQUAL";
    case OpCode::NOT_EQUAL:
        return "BANG_EQUAL";
    case OpCode::NOT:
        return "NOT";
    case OpCode::AND:
        return "AND";
    case OpCode::OR:
        return "OR";
    case OpCode::MOVE:
        return "MOVE";
    case jl::OpCode::LABEL:
        return "LABEL";
    case jl::OpCode::JMP:
        return "JMP";
    case jl::OpCode::JMP_UNLESS:
        return "JMP_UNLESS";
    default:
        unimplemented();
        return "UNKNOWN";
    }
}

jl::OperatorCategory jl::get_category(const OpCode& opcode)
{
    switch (opcode) {
    case OpCode::ADD:
    case OpCode::MINUS:
    case OpCode::STAR:
    case OpCode::MODULUS:
    case OpCode::SLASH:
        return OperatorCategory::ARITHAMETIC;
    case OpCode::GREATER:
    case OpCode::LESS:
    case OpCode::GREATER_EQUAL:
    case OpCode::LESS_EQUAL:
    case OpCode::EQUAL:
    case OpCode::NOT_EQUAL:
        return OperatorCategory::COMPARISON;
    case OpCode::NOT:
    case OpCode::AND:
    case OpCode::OR:
        return OperatorCategory::BOOLEAN;
    case OpCode::MOVE:
    case OpCode::RETURN:
    case jl::OpCode::JMP_UNLESS:
    case jl::OpCode::LABEL:
    case jl::OpCode::JMP:
        return OperatorCategory::OTHER;
        break;
    }
}