#pragma once

namespace jl {

enum class OpCode {
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
    ASSIGN,
    
    RETURN,
};

const char* to_string(OpCode opcode);

}