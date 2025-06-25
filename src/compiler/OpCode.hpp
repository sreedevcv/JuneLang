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
    MOVE,
    LABEL,
    JMP,
    JMP_UNLESS,
    RETURN,
    PUSH,
    POP,
    CALL
};

enum class OperatorCategory {
    ARITHAMETIC,
    COMPARISON,
    BOOLEAN,
    OTHER,
};

const char* to_string(OpCode opcode);
OperatorCategory get_category(const OpCode& opcode);

}