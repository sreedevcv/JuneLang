#include "VM.hpp"

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "ErrorHandler.hpp"
#include "Operand.hpp"

static bool is_number(const jl::Operand& op)
{
    const auto type = jl::get_type(op);

    return (type == jl::OperandType::INT || type == jl::OperandType::FLOAT)
        ? true
        : false;
}

static const jl::Operand& get_temp_var_data(
    const jl::Operand& op1,
    const std::vector<jl::Operand>& temp_vars)
{
    const auto var = std::get<jl::TempVar>(op1);
    return temp_vars[var.idx];
}

// Does the operation and return the value(double | int) as Operand
//
template <typename Op>
static const jl::Operand binary_operation(
    const jl::Operand& op1,
    const jl::Operand& op2,
    Op bin_oper,
    const std::vector<jl::Operand>& temp_vars,
    uint32_t line)
{
    const auto& left = jl::get_type(op1) == jl::OperandType::TEMP
        ? get_temp_var_data(op1, temp_vars)
        : op1;
    const auto& right = jl::get_type(op2) == jl::OperandType::TEMP
        ? get_temp_var_data(op2, temp_vars)
        : op2;

    auto file = std::string("temp");
    if (!is_number(left) && !is_number(right)) {
        jl::ErrorHandler::error(file, "vm", "binary expression", line, "Left and right operands must be a number", 0);
        std::exit(1);
    }

    const auto type1 = jl::get_type(left);
    const auto type2 = jl::get_type(right);

    if (type1 == jl::OperandType::FLOAT || type2 == jl::OperandType::FLOAT) {
        const double& l = type1 == jl::OperandType::FLOAT
            ? std::get<double>(left)
            : std::get<int>(left);
        const double& r = type2 == jl::OperandType::FLOAT
            ? std::get<double>(right)
            : std::get<int>(right);

        return jl::Operand { bin_oper(l, r) };
    } else {
        const int& l = std::get<int>(left);
        const int& r = std::get<int>(right);

        return jl::Operand { bin_oper(l, r) };
    }
}

std::pair<jl::VM::InterpretResult, std::vector<jl::Operand>> jl::VM::run(const Chunk& chunk)
{
    std::vector<Operand> temp_vars { chunk.get_max_allocated_temps() };

    for (const auto& ir : chunk.get_ir()) {
        Operand result;

        switch (ir.opcode) {
        case OpCode::ADD: {
            result = binary_operation(
                ir.op1,
                ir.op2,
                std::plus<> {},
                temp_vars,
                0);
        } break;
        case OpCode::MINUS: {
            result = binary_operation(
                ir.op1,
                ir.op2,
                std::minus<> {},
                temp_vars,
                0);
        } break;
        case OpCode::STAR: {
            result = binary_operation(
                ir.op1,
                ir.op2,
                std::multiplies<> {},
                temp_vars,
                0);
        } break;
        case OpCode::SLASH: {
            result = binary_operation(
                ir.op1,
                ir.op2,
                std::divides<> {},
                temp_vars,
                0);
        } break;
        case OpCode::GREATER: {
            result = binary_operation(
                ir.op1,
                ir.op2,
                std::greater<> {},
                temp_vars,
                0);
        } break;
        case OpCode::LESS: {
            result = binary_operation(
                ir.op1,
                ir.op2,
                std::less<> {},
                temp_vars,
                0);
        } break;
        case OpCode::GREATER_EQUAL: {
            result = binary_operation(
                ir.op1,
                ir.op2,
                std::greater_equal<> {},
                temp_vars,
                0);
        } break;
        case OpCode::LESS_EQUAL: {
            result = binary_operation(
                ir.op1,
                ir.op2,
                std::less_equal<> {},
                temp_vars,
                0);
        } break;
        case OpCode::EQUAL: {
            result = binary_operation(
                ir.op1,
                ir.op2,
                std::equal_to<> {},
                temp_vars,
                0);
        } break;
        case OpCode::NOT_EQUAL: {
            result = binary_operation(
                ir.op1,
                ir.op2,
                std::not_equal_to<> {},
                temp_vars,
                0);
        } break;
            // case OpCode::NOT: {
            //     result = binary_operation(
            //         ir.op1,
            //         ir.op2,
            //         std::plus<> {},
            //         temp_vars,
            //         0);
            // } break;
            // case OpCode::AND: {
            //     result = binary_operation(
            //         ir.op1,
            //         ir.op2,
            //         std::plus<> {},
            //         temp_vars,
            //         0);
            // } break;
            // case OpCode::OR: {
            //     result = binary_operation(
            //         ir.op1,
            //         ir.op2,
            //         std::plus<> {},
            //         temp_vars,
            //         0);
            // } break;
        case OpCode::ASSIGN: {
            result = jl::get_type(ir.op1) == jl::OperandType::TEMP
                ? get_temp_var_data(ir.op1, temp_vars)
                : ir.op1;
        } break;
        default:
            unimplemented();
            break;
        }

        temp_vars[ir.dest.idx] = result;
    }

    return {InterpretResult::OK, temp_vars};
}
