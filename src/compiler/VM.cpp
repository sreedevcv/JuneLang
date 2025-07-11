#include "VM.hpp"

#include <climits>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iostream>
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
static const jl::Operand execute_bitwise_and_modulus(
    const jl::Operand& op1,
    const jl::Operand& op2,
    Op bin_oper)
{
    const int& l = std::get<int>(op1);
    const int& r = std::get<int>(op2);

    return jl::Operand { bin_oper(l, r) };
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
        const jl::ptr_type addr1 = jl::is_pure_ptr(type1)
            ? std::get<jl::PtrVar>(op1).offset
            : std::get<int>(op1) * jl::size_of_type(*jl::from_ptr(type2));

        const auto addr2 = jl::is_pure_ptr(type2)
            ? std::get<jl::PtrVar>(op2).offset
            : std::get<int>(op2) * jl::size_of_type(*jl::from_ptr(type1));

        const auto ptr_type = jl::is_pure_ptr(type1) ? type1 : type2;

        return jl::Operand { jl::PtrVar { .offset = bin_oper(addr1, addr2), .type = ptr_type } };
    } else if (type1 == jl::OperandType::CHAR && type2 == jl::OperandType::CHAR) {
        const auto& l = std::get<char>(op1);
        const auto& r = std::get<char>(op2);

        return jl::Operand { bin_oper(l, r) };
    } else {
        unimplemented();
        return jl::Operand {};
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
    case jl::OpCode::ADD:
        result = execute_arithametic(op1, op2, std::plus<> {});
        break;
    case jl::OpCode::MINUS:
        result = execute_arithametic(op1, op2, std::minus<> {});
        break;
    case jl::OpCode::STAR:
        result = execute_arithametic(op1, op2, std::multiplies<> {});
        break;
    case jl::OpCode::SLASH:
        result = execute_arithametic(op1, op2, std::divides<> {});
        break;
    case jl::OpCode::GREATER:
        result = execute_arithametic(op1, op2, std::greater<> {});
        break;
    case jl::OpCode::LESS:
        result = execute_arithametic(op1, op2, std::less<> {});
        break;
    case jl::OpCode::GREATER_EQUAL:
        result = execute_arithametic(op1, op2, std::greater_equal<> {});
        break;
    case jl::OpCode::LESS_EQUAL:
        result = execute_arithametic(op1, op2, std::less_equal<> {});
        break;
    case jl::OpCode::EQUAL:
        result = execute_arithametic(op1, op2, std::equal_to<> {});
        break;
    case jl::OpCode::NOT_EQUAL:
        result = execute_arithametic(op1, op2, std::not_equal_to<> {});
        break;
    case jl::OpCode::MODULUS:
        result = execute_bitwise_and_modulus(op1, op2, std::modulus<> {});
        break;
    case jl::OpCode::BIT_AND:
        result = execute_bitwise_and_modulus(op1, op2, std::bit_and<> {});
        break;
    case jl::OpCode::BIT_OR:
        result = execute_bitwise_and_modulus(op1, op2, std::bit_or<> {});
        break;
    case jl::OpCode::BIT_XOR:
        result = execute_bitwise_and_modulus(op1, op2, std::bit_xor<> {});
        break;
    case jl::OpCode::AND:
        result = execute_boolean(op1, op2, std::logical_and<> {});
        break;
    case jl::OpCode::OR:
        result = execute_boolean(op1, op2, std::logical_or<> {});
        break;
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
    std::vector<Operand>& temp_vars,
    DataSection& data_section)
{
    const auto& irs = chunk.get_ir();
    const auto locations = fill_labels(irs, chunk.get_max_labels());
    uint32_t pc = 0;

    while (pc < irs.size()) {
        const auto& ir = irs[pc];
        if (debug_run)
            debug_print(chunk, pc, ir, temp_vars);
        pc = execute_ir(ir, pc, chunk, chunk_map, temp_vars, locations, data_section);
    }

    return InterpretResult::OK;
}

uint32_t jl::VM::execute_ir(
    Ir ir,
    uint32_t pc,
    const Chunk& chunk,
    const std::map<std::string, Chunk>& chunk_map,
    std::vector<Operand>& temp_vars,
    const std::vector<uint32_t> locations,
    DataSection& data_section)
{
    if (ir.opcode() == OpCode::CALL) {
        const auto& cir = ir.call();
        const auto& func_chunk = chunk_map.at(cir.func_name);
        temp_vars[cir.return_var.idx] = run_function(cir, func_chunk, chunk_map, data_section, temp_vars);
        return pc + 1;
    }

    switch (ir.type()) {
    case Ir::BINARY:
        handle_binary_ir(ir, temp_vars);
        return pc + 1;
    case Ir::UNARY:
        handle_unary_ir(ir, temp_vars, data_section);
        return pc + 1;
    case Ir::JUMP_STORE:
    case Ir::CONTROL:
        return handle_control_ir(pc, ir, temp_vars, locations, data_section);
    default:
        unimplemented();
    }

    return pc + 1;
}

void jl::VM::handle_binary_ir(const Ir& ir, std::vector<Operand>& temp_vars)
{
    Operand result;
    const auto& binar_ir = ir.binary();

    const auto& left = temp_vars[binar_ir.op1.idx];
    const auto& right = temp_vars[binar_ir.op2.idx];

    const auto type1 = jl::get_type(left);
    const auto type2 = jl::get_type(right);

    switch (jl::get_category(binar_ir.opcode)) {
    case jl::OperatorCategory::ARITHAMETIC:
    case jl::OperatorCategory::COMPARISON:
    case jl::OperatorCategory::BOOLEAN:
    case OperatorCategory::BITWISE_AND_MODULUS:
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
    case jl::OpCode::BIT_NOT: {
        result = ~std::get<int>(operand);
    } break;
    case jl::OpCode::LOAD: {
        const auto& op = get_nested_data(operand, temp_vars);
        const auto offset = std::get<PtrVar>(op).offset;

        switch (get_type(op)) {
        case OperandType::CHAR_PTR: {
            const auto data = data_section.read_data<char>(offset);
            result = Operand { data };
        } break;
        case OperandType::INT_PTR: {
            const auto data = data_section.read_data<int_type>(offset);
            result = Operand { data };
        } break;
        case OperandType::FLOAT_PTR: {
            const auto data = data_section.read_data<float_type>(offset);
            result = Operand { data };
        } break;
        case OperandType::BOOL_PTR: {
            const auto data = data_section.read_data<bool>(offset);
            result = Operand { data };
        } break;
        default:
            unimplemented();
            break;
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
    const std::vector<uint32_t>& label_locations,
    DataSection& data_section)
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
        const auto& condition = temp_vars[ir.jump().data.idx];
        // Evaluate and jump
        if (std::get<bool>(condition) == false) {
            const auto label = std::get<int>(ir.jump().target);
            return label_locations[label];
        } else {
            return pc + 1;
        }
    } break;
    case OpCode::RETURN: {
        const auto& operand = ir.control().data;
        const auto& data = jl::get_type(operand) == jl::OperandType::TEMP
            ? get_temp_var_data(operand, temp_vars)
            : operand;
        m_stack.push(data);
        return UINT_MAX;
    } break;
    case OpCode::STORE: {
        auto data = temp_vars[ir.jump().data.idx];
        const auto addr = get_nested_data(ir.jump().target, temp_vars);

        switch (get_type(addr)) {
        case OperandType::CHAR_PTR:
            data_section.set_data(std::get<PtrVar>(addr).offset, std::get<char>(data));
            break;
        case OperandType::INT_PTR:
            data_section.set_data(std::get<PtrVar>(addr).offset, std::get<int_type>(data));
            break;
        case OperandType::FLOAT_PTR:
            data_section.set_data(std::get<PtrVar>(addr).offset, std::get<float_type>(data));
            break;
        case OperandType::BOOL_PTR:
            data_section.set_data(std::get<PtrVar>(addr).offset, std::get<bool>(data));
            break;
        default:
            unimplemented();
            break;
        }
    } break;
    default:
        unimplemented();
    }

    return pc + 1;
}

std::pair<jl::VM::InterpretResult, std::vector<jl::Operand>> jl::VM::interactive_execute(
    const Chunk& chunk,
    const std::map<std::string, Chunk>& chunk_map,
    DataSection& data_section)
{
    std::vector<Operand> temp_vars(chunk.get_max_allocated_temps());
    debug_run = true;
    const auto result = run(chunk, chunk_map, temp_vars, data_section);
    return { result, temp_vars };
}

void jl::VM::debug_print(
    const Chunk& chunk,
    uint32_t pc,
    const Ir& ir,
    const std::vector<Operand>& temp_vars)
{
    std::cout << "================================================================================\n";

    std::cout << pc << " >";
    chunk.print_ir(std::cout, ir);
    std::cout << '\n';

    for (int i = 0; i < temp_vars.size(); i++) {
        if (i % 8 == 0)
            std::cout << '\n';

        std::cout << i << ": ";
        const auto& op = temp_vars[i];
        std::cout << to_string(op) << "\t";
    }
    std::cout << '\n';

    char enter;
    std::cin.get();
}

jl::Operand jl::VM::run_function(
    const CallIr& ir,
    const Chunk& func_chunk,
    const std::map<std::string, Chunk>& chunk_map,
    DataSection& data_section,
    const std::vector<Operand>& temp_vars)
{
    if (!func_chunk.extern_symbol) {
        std::vector<Operand> stack_vars { func_chunk.get_max_allocated_temps() };
        for (int i = 0; i < ir.args.size(); i++) {
            // First temp var will always be the fucntion itself
            stack_vars[i + 1] = temp_vars[ir.args[i].idx];
        }

        run(func_chunk, chunk_map, stack_vars, data_section);
        const auto ret_value = m_stack.top();

        return ret_value;
    } else {
        std::vector<Operand> args;
        for (int i = 0; i < ir.args.size(); i++) {
            args.push_back(temp_vars[ir.args[i].idx]);
        }

        Operand ret_value = m_ffi.call(
            *func_chunk.extern_symbol,
            args,
            func_chunk.return_type,
            data_section.data());

        return ret_value;
    }
}