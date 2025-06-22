#include "VM.hpp"

#include <functional>
#include <vector>

#include "OpCode.hpp"
#include "Operand.hpp"
#include "Utils.hpp"

static const jl::Operand& get_temp_var_data(
    const jl::Operand& op1,
    const std::vector<jl::Operand>& temp_vars)
{
    const auto var = std::get<jl::TempVar>(op1);
    return temp_vars[var.idx];
}

template <typename Op>
static const jl::Operand execute_arithametic(
    const jl::Operand& op1,
    const jl::Operand& op2,
    Op bin_oper)
{
    const auto type1 = jl::get_type(op1);
    const auto type2 = jl::get_type(op2);

    if (type1 == jl::OperandType::FLOAT || type2 == jl::OperandType::FLOAT) {
        const double& l = type1 == jl::OperandType::FLOAT
            ? std::get<double>(op1)
            : std::get<int>(op1);
        const double& r = type2 == jl::OperandType::FLOAT
            ? std::get<double>(op2)
            : std::get<int>(op2);

        return jl::Operand { bin_oper(l, r) };
    } else {
        const int& l = std::get<int>(op1);
        const int& r = std::get<int>(op2);

        return jl::Operand { bin_oper(l, r) };
    }
}

template <typename Op>
static const jl::Operand execute_boolean(
    const jl::Operand& op1,
    const jl::Operand& op2,
    Op bin_oper)
{
    const auto l = std::get<bool>(op1);
    const auto r = std::get<bool>(op2);
    return jl::Operand { bin_oper(l, r) };
}

static const jl::Operand do_arithametic(
    const jl::Operand& op1,
    const jl::Operand& op2,
    jl::OpCode opcode)
{
    jl::Operand result;
    switch (opcode) {
    case jl::OpCode::ADD: {
        result = execute_arithametic(
            op1,
            op2,
            std::plus<> {});
    } break;
    case jl::OpCode::MINUS: {
        result = execute_arithametic(
            op1,
            op2,
            std::minus<> {});
    } break;
    case jl::OpCode::STAR: {
        result = execute_arithametic(
            op1,
            op2,
            std::multiplies<> {});
    } break;
    case jl::OpCode::SLASH: {
        result = execute_arithametic(
            op1,
            op2,
            std::divides<> {});
    } break;
    case jl::OpCode::GREATER: {
        result = execute_arithametic(
            op1,
            op2,
            std::greater<> {});
    } break;
    case jl::OpCode::LESS: {
        result = execute_arithametic(
            op1,
            op2,
            std::less<> {});
    } break;
    case jl::OpCode::GREATER_EQUAL: {
        result = execute_arithametic(
            op1,
            op2,
            std::greater_equal<> {});
    } break;
    case jl::OpCode::LESS_EQUAL: {
        result = execute_arithametic(
            op1,
            op2,
            std::less_equal<> {});
    } break;
    case jl::OpCode::EQUAL: {
        result = execute_arithametic(
            op1,
            op2,
            std::equal_to<> {});
    } break;
    case jl::OpCode::NOT_EQUAL: {
        result = execute_arithametic(
            op1,
            op2,
            std::not_equal_to<> {});
    } break;
    case jl::OpCode::AND: {
        result = execute_boolean(
            op1,
            op2,
            std::logical_and<> {});
    } break;
    case jl::OpCode::OR: {
        result = execute_boolean(
            op1,
            op2,
            std::logical_or<> {});
    } break;

    default:
        unimplemented();
    }

    return result;
}

std::pair<jl::VM::InterpretResult, std::vector<jl::Operand>> jl::VM::run(const Chunk& chunk)
{
    std::vector<Operand> temp_vars { chunk.get_max_allocated_temps() };

    for (const auto& ir : chunk.get_ir()) {
        switch (ir.type()) {
        case Ir::BINARY:
            handle_binary_ir(ir, temp_vars);
            break;
        case Ir::UNARY:
            handle_unary_ir(ir, temp_vars);
            break;
        default:
            unimplemented();
        }
    }

    return { InterpretResult::OK, temp_vars };
}

void jl::VM::handle_binary_ir(const Ir& ir, std::vector<Operand>& temp_vars)
{
    Operand result;
    const auto& binar_ir = ir.binary();

    const auto& left = jl::get_type(binar_ir.op1) == jl::OperandType::TEMP
        ? get_temp_var_data(binar_ir.op1, temp_vars)
        : binar_ir.op1;
    const auto& right = jl::get_type(binar_ir.op2) == jl::OperandType::TEMP
        ? get_temp_var_data(binar_ir.op2, temp_vars)
        : binar_ir.op2;

    const auto type1 = jl::get_type(left);
    const auto type2 = jl::get_type(right);

    switch (jl::get_category(binar_ir.opcode)) {
    case jl::OperatorCategory::ARITHAMETIC:
    case jl::OperatorCategory::COMPARISON:
    case jl::OperatorCategory::BOOLEAN:
        result = do_arithametic(left, right, binar_ir.opcode);
        break;
    case jl::OperatorCategory::OTHER:
        unimplemented();
        break;
    }

    temp_vars[ir.dest().idx] = result;
}

void jl::VM::handle_unary_ir(const Ir& ir, std::vector<Operand>& temp_vars)
{
    Operand result;
    const auto& unary_ir = ir.unary();
    const auto operand = jl::get_type(unary_ir.operand) == jl::OperandType::TEMP
        ? get_temp_var_data(unary_ir.operand, temp_vars)
        : unary_ir.operand;

    switch (ir.opcode()) {
    case OpCode::MOVE: {
        result = operand;
    } break;
    case jl::OpCode::NOT: {
        result = !std::get<bool>(operand);
    } break;
    case jl::OpCode::MINUS: {
        if (get_type(operand) == OperandType::INT) {
            result = -1 * std::get<int>(operand);
        } else if (get_type(operand) == OperandType::FLOAT) {
            result = -1.0 * std::get<double>(operand);
        } else {
            unimplemented();
        }
    } break;
    default:
        unimplemented();
    }

    temp_vars[ir.dest().idx] = result;
}
