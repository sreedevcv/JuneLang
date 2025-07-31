#pragma once

#include "OpCode.hpp"
#include "Operand.hpp"

#include <cstdint>
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
    OperandType type;
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

struct JumpIr {
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

struct LoadStoreIr {
    OpCode opcode;
    TempVar addr;
    TempVar reg;
    uint32_t size;
};

struct Ir {
    std::variant<UnaryIr, BinaryIr, ControlIr, JumpIr, CallIr, TypeCastIr, LoadStoreIr> data;

    enum Type {
        UNARY,
        BINARY,
        CONTROL,
        JUMP_STORE,
        CALL,
        TYPE_CAST,
        LOAD_STORE,
    };

    Type type() const;
    const BinaryIr& binary() const;
    const UnaryIr& unary() const;
    const ControlIr& control() const;
    const JumpIr& jump() const;
    const OpCode& opcode() const;
    const TempVar& dest() const;
    const CallIr& call() const;
    const TypeCastIr& cast() const;
    const LoadStoreIr& load_store() const;
};

Ir::Type get_type(const Ir& ir);

}