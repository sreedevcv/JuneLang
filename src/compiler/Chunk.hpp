#pragma once

#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vector>

#include "OpCode.hpp"
#include "Operand.hpp"

namespace jl {

struct Ir {
    OpCode opcode;
    Operand op1;
    Operand op2;
    TempVar dest;
};

class Chunk {
public:
    std::string disassemble() const;
    TempVar write(OpCode opcode,
        Operand op1,
        Operand op2);
    void write_with_dest(OpCode opcode,
        Operand op1,
        Operand op2,
        TempVar dest);
    TempVar store_variable(const std::string& var_name);
    std::optional<TempVar> look_up_variable(const std::string& var_name) const;

private:
    std::vector<Ir> m_ops;
    uint32_t m_temp_var_count { 0 };
    std::unordered_map<std::string, TempVar> m_variable_map;

    TempVar create_temp_var();
    void output_var_map(std::ostream& in) const;
};

}