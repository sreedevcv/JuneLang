#pragma once

#include "Ir.hpp"
#include "Operand.hpp"
#include <cstdint>
#include <stack>
#include <vector>

namespace jl {

class FlatVM {
public:
    enum InterpretResult {
        OK,
        COMPILER_ERROR,
        RUNTIME_ERROR,
    };

    FlatVM();
    const uint32_t stack_size { 100 };
    std::pair<InterpretResult, std::vector<Operand>> run(const std::vector<Ir>& irs);

private:
    std::stack<Operand> m_stack;
    std::vector<Operand>* m_registers;
    std::stack<std::vector<Operand>> m_reg_stack;
    Operand ret_val;

    void handle_binary_ir(const Ir& ir);
    void handle_unary_ir(const Ir& ir);
    uint32_t handle_control_ir(const uint32_t pc, const Ir& ir);
};

}