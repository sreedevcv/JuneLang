#include "OpCode.hpp"
#include <print>

const char* jl::to_string(OpCode opcode)
{
    switch (opcode) {
    case OpCode::RETURN:
        return "RETURN";
    case OpCode::CONSTANT:
        return "CONSTANT";
    default:
        std::println("Unimplemented OpCode\n");
        return "UNKNOWN";
    }
}