#pragma once

#include "OpCode.hpp"
#include "Operand.hpp"

#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace jl {

struct BinaryIr {
    OpCode opcode;
    TempVar op1;
    TempVar op2;
    TempVar dest;
};

struct UnaryIr {
    OpCode opcode;
    Operand operand;
    TempVar dest;
};

struct ControlIr {
    OpCode opcode;
    Operand data;
};

struct JumpStoreIr {
    OpCode opcode;
    TempVar data;
    Operand target;
};

struct CallIr {
    OpCode opcode;
    TempVar func_var;
    std::string func_name;
    std::vector<TempVar> args;
    std::optional<std::string> extern_symbol;
    TempVar return_var;
};

struct TypeCastIr {
    OpCode opcode;
    TempVar dest;
    TempVar source;
    OperandType from;
    OperandType to;
};

struct Ir {
    std::variant<UnaryIr, BinaryIr, ControlIr, JumpStoreIr, CallIr, TypeCastIr> data;

    enum Type {
        UNARY,
        BINARY,
        CONTROL,
        JUMP_STORE,
        CALL,
        TYPE_CAST,
    };

    Type type() const;
    const BinaryIr& binary() const;
    const UnaryIr& unary() const;
    const ControlIr& control() const;
    const JumpStoreIr& jump() const;
    const OpCode& opcode() const;
    const TempVar& dest() const;
    const CallIr& call() const;
    const TypeCastIr& cast() const;
};

Ir::Type get_type(const Ir& ir);

}