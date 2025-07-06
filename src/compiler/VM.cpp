#include "VM.hpp"

#include <climits>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <print>
#include <vector>

#include "Ir.hpp"
#include "OpCode.hpp"
#include "Operand.hpp"
#include "Utils.hpp"

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
    } else if (type1 == jl::OperandType::INT && type2 == jl::OperandType::INT) {
        const int& l = std::get<int>(op1);
        const int& r = std::get<int>(op2);

        return jl::Operand { bin_oper(l, r) };

    } else if (jl::is_ptr(type1) || jl::is_ptr(type2)) {

        constexpr auto add_ptr = [](jl::OperandType type, const jl::Operand& op) {
            jl::ptr_type addr = 0;
            switch (type) {
            case jl::OperandType::CHAR_PTR:
                addr += std::get<jl::PtrVar>(op).offset;
                break;
            case jl::OperandType::INT:
                addr += std::get<int>(op);
                break;
            default:
                unimplemented();
            }

            return addr;
        };

        const auto addr1 = add_ptr(type1, op1);
        const auto addr2 = add_ptr(type2, op2);

        return jl::Operand { jl::PtrVar { .offset = bin_oper(addr1, addr2) } };
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

std::pair<jl::VM::InterpretResult, std::vector<jl::Operand>> jl::VM::run(
    const Chunk& chunk,
    const std::map<std::string, Chunk>& chunk_map,
    DataSection& data_section)
{
    std::vector<Operand> temp_vars(chunk.get_max_allocated_temps());
    const auto result = run(chunk, chunk_map, temp_vars, data_section);

    return { result, temp_vars };
}

jl::VM::InterpretResult jl::VM::run(
    const Chunk& chunk,
    const std::map<std::string, Chunk>& chunk_map,
    std::vector<Operand>& temp_vars, DataSection& data_section)
{
    const auto& irs = chunk.get_ir();
    const auto locations = fill_labels(irs, chunk.get_max_labels());
    uint32_t pc = 0;

    while (pc < irs.size()) {
        const auto& ir = irs[pc];

        if (ir.opcode() == OpCode::CALL) {
            const auto& cir = ir.call();
            const auto& func_chunk = chunk_map.at(cir.func_name);

            std::vector<Operand> stack_vars { func_chunk.get_max_allocated_temps() };
            for (int i = 0; i < cir.args.size(); i++) {
                // First temp var will always be the fucntion itself
                stack_vars[i + 1] = get_nested_data(cir.args[0], temp_vars);
            }

            run(func_chunk, chunk_map, stack_vars, data_section);
            const auto ret_value = m_stack.top();
            temp_vars[cir.dest.idx] = ret_value;
            pc += 1;
            continue;
        }

        switch (ir.type()) {
        case Ir::BINARY:
            handle_binary_ir(ir, temp_vars);
            pc += 1;
            break;
        case Ir::UNARY:
            handle_unary_ir(ir, temp_vars, data_section);
            pc += 1;
            break;
        case Ir::JUMP:
        case Ir::CONTROL:
            pc = handle_control_ir(pc, ir, temp_vars, locations);
            break;
        default:
            unimplemented();
        }
    }

    return InterpretResult::OK;
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

void jl::VM::handle_unary_ir(
    const Ir& ir,
    std::vector<Operand>& temp_vars,
    DataSection& data_section)
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
    case jl::OpCode::LOAD: {
        const auto& op = get_nested_data(operand, temp_vars);
        if (get_type(op) == OperandType::CHAR_PTR) {
            const auto offset = std::get<PtrVar>(op).offset;
            const auto data = data_section.read_data<char>(offset);
            result = Operand { data };
        } else {
            unimplemented();
        }
    } break;
    default:
        unimplemented();
    }

    temp_vars[ir.dest().idx] = result;
}

std::vector<uint32_t> jl::VM::fill_labels(const std::vector<Ir>& irs, uint32_t max_labels) const
{
    if (max_labels == 0) {
        return {};
    }

    std::vector<uint32_t> locations;
    locations.resize(max_labels);

    for (int i = 0; i < irs.size(); i++) {
        const auto& ir = irs[i];

        if (ir.opcode() == OpCode::LABEL) {
            locations[std::get<int>(ir.control().data)] = i;
        }
    }

    return locations;
}

uint32_t jl::VM::handle_control_ir(
    const uint32_t pc,
    const Ir& ir, std::vector<Operand>& temp_vars,
    const std::vector<uint32_t>& label_locations)
{
    switch (ir.opcode()) {
    case OpCode::LABEL:
        break;
    case OpCode::HALT:
        std::println("[Halting...]");
        std::exit(1);
        break;
    case OpCode::JMP: {
        const auto label = std::get<int>(ir.control().data);
        return label_locations[label];
    } break;
    case OpCode::JMP_UNLESS: {
        const auto condition = get_nested_data(ir.jump().data, temp_vars);
        // Evaluate and jump
        if (std::get<bool>(condition) == false) {
            const auto label = std::get<int>(ir.jump().label);
            return label_locations[label];
        } else {
            return pc + 1;
        }
    } break;
    // case OpCode::PUSH: {
    //     const auto& operand = ir.control().data;
    //     const auto& data = jl::get_type(operand) == jl::OperandType::TEMP
    //         ? get_temp_var_data(operand, temp_vars)
    //         : operand;
    //     m_stack.push(data);
    // } break;
    // case OpCode::POP: {
    //     const auto data = m_stack.top();
    //     m_stack.pop();
    //     const auto temp_var = std::get<TempVar>(ir.control().data);
    //     temp_vars[temp_var.idx] = data;
    // } break;
    case OpCode::RETURN: {
        const auto& operand = ir.control().data;
        const auto& data = jl::get_type(operand) == jl::OperandType::TEMP
            ? get_temp_var_data(operand, temp_vars)
            : operand;
        m_stack.push(data);
        return UINT_MAX;
    } break;

    default:
        unimplemented();
    }

    return pc + 1;
}
