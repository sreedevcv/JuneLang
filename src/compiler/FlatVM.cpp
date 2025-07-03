#include "FlatVM.hpp"

#include <cstdint>
#include <cstdlib>
#include <functional>
#include <print>
#include <vector>

#include "Ir.hpp"
#include "OpCode.hpp"
#include "Operand.hpp"
#include "Utils.hpp"

jl::FlatVM::FlatVM()
{
    m_reg_stack.push({});

    m_registers = &m_reg_stack.top();
    m_registers->resize(stack_size);
}

static const jl::Operand& get_temp_var_data(
    const jl::Operand& operand,
    const std::vector<jl::Operand>& temp_vars)
{
    const auto var = std::get<jl::TempVar>(operand);
    return temp_vars[var.idx];
}

static const jl::Operand& get_nested_data(
    const jl::Operand& operand,
    const std::vector<jl::Operand>& temp_vars)
{
    if (jl::get_type(operand) == jl::OperandType::TEMP) {
        return get_temp_var_data(operand, temp_vars);
    } else {
        return operand;
    }
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

std::pair<jl::FlatVM::InterpretResult, std::vector<jl::Operand>>
jl::FlatVM::run(const std::vector<Ir>& irs)
{
    uint32_t pc = 0;

    while (pc < irs.size()) {
        const auto& ir = irs[pc];

        if (ir.opcode() == OpCode::CALL) {
            m_reg_stack.push({});
            m_registers = &m_reg_stack.top();
            m_registers->resize(stack_size);
            m_stack.push(static_cast<int>(pc + 1));
            continue;
        }

        switch (ir.type()) {
        case Ir::BINARY:
            handle_binary_ir(ir);
            pc += 1;
            break;
        case Ir::UNARY:
            handle_unary_ir(ir);
            pc += 1;
            break;
        case Ir::JUMP:
        case Ir::CONTROL:
            pc = handle_control_ir(pc, ir);
            break;
        default:
            unimplemented();
        }
    }

    return { InterpretResult::OK, {} };
}

void jl::FlatVM::handle_binary_ir(const Ir& ir)
{
    Operand result;
    const auto& binar_ir = ir.binary();

    const auto& left = jl::get_type(binar_ir.op1) == jl::OperandType::TEMP
        ? get_temp_var_data(binar_ir.op1, *m_registers)
        : binar_ir.op1;
    const auto& right = jl::get_type(binar_ir.op2) == jl::OperandType::TEMP
        ? get_temp_var_data(binar_ir.op2, *m_registers)
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

    m_registers->at(ir.dest().idx) = result;
}

void jl::FlatVM::handle_unary_ir(const Ir& ir)
{
    Operand result;
    const auto& unary_ir = ir.unary();
    const auto operand = jl::get_type(unary_ir.operand) == jl::OperandType::TEMP
        ? get_temp_var_data(unary_ir.operand, *m_registers)
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

    m_registers->at(ir.dest().idx) = result;
}

uint32_t jl::FlatVM::handle_control_ir(
    const uint32_t pc,
    const Ir& ir)
{
    switch (ir.opcode()) {
    case OpCode::LABEL:
        break;
    case OpCode::HALT:
        std::println("[Halting...]");
        std::exit(1);
        break;
    case OpCode::JMP: {
        const auto loc = std::get<int>(ir.control().data);
        return loc;
    } break;
    case OpCode::JMP_UNLESS: {
        const auto condition = get_nested_data(ir.jump().data, *m_registers);
        // Evaluate and jump
        if (std::get<bool>(condition) == false) {
            const auto loc = std::get<int>(ir.jump().label);
            return loc;
        } else {
            return pc + 1;
        }
    } break;
    case OpCode::PUSH: {
        const auto& operand = ir.control().data;
        const auto& data = jl::get_type(operand) == jl::OperandType::TEMP
            ? get_temp_var_data(operand, *m_registers)
            : operand;
        m_stack.push(data);
    } break;
    case OpCode::POP: {
        const auto data = m_stack.top();
        m_stack.pop();
        const auto temp_var = std::get<TempVar>(ir.control().data);
        m_registers->at(temp_var.idx) = data;
    } break;
    case OpCode::RETURN: {
        const auto& operand = ir.control().data;
        const auto& data = jl::get_type(operand) == jl::OperandType::TEMP
            ? get_temp_var_data(operand, *m_registers)
            : operand;
        // Put the return value in the first register
        // *m_registers[0] = data;
        ret_val = data;
    } break;

    default:
        unimplemented();
    }

    return pc + 1;
}
