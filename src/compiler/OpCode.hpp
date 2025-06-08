#pragma once

namespace jl {

enum class OpCode {
    RETURN,
    CONSTANT,
};

const char* to_string(OpCode opcode);

}