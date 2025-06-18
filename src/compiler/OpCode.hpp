#pragma once

namespace jl {

enum class OpCode {
    RETURN,
    CONSTANT,
    NEGATE,
    ADD,
    MINUS,
    STAR,
    SLASH,
    GREATER,
    LESS,
    GREATER_EQUAL,
    LESS_EQUAL,
    MODULUS,
    EQUAL,
    NOT_EQUAL,
    NOT,
    AND,
    OR,

    // 
    ASSIGN
};

const char* to_string(OpCode opcode);

}