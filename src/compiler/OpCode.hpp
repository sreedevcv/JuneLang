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
    BIT_AND,
    BIT_OR,
    BIT_XOR,
    BIT_NOT,
    //
    MOVE,
    LABEL,
    JMP,
    JMP_UNLESS,
    RETURN,
    PUSH,
    POP,
    CALL,
    LOAD,
    STORE,
    HALT // For runtime error
};

enum class OperatorCategory {
    ARITHAMETIC,
    COMPARISON,
    BOOLEAN,
    BITWISE_AND_MODULUS,
    OTHER,
};

const char* to_string(OpCode opcode);
OperatorCategory get_category(const OpCode& opcode);

}