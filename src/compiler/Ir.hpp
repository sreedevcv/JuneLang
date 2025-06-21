#pragma once

#include "OpCode.hpp"
#include "Operand.hpp"
#include <variant>

namespace jl {

struct BinaryIr {
    OpCode opcode;
    Operand op1;
    Operand op2;
    TempVar dest;
};

struct UnaryIr {
    OpCode opcode;
    Operand operand;
    TempVar dest;
};

struct Ir {
    std::variant<UnaryIr, BinaryIr> data;

    enum Type {
        UNARY,
        BINARY,
    };

    Type type() const;
    const BinaryIr& binary() const;
    const UnaryIr& unary() const;
    const OpCode& opcode() const;
    const TempVar& dest() const;
};

Ir::Type get_type(const Ir& ir);

}