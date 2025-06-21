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
    case OpCode::ASSIGN:
        return "ASSIGN";
    default:
        unimplemented();
        return "UNKNOWN";
    }
}