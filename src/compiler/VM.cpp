#include "VM.hpp"

#include <climits>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <print>
#include <type_traits>
#include <vector>

#include "Ir.hpp"
#include "OpCode.hpp"
#include "Operand.hpp"
#include "Utils.hpp"

jl::VM::VM(std::map<std::string, Chunk>& m_chunk_map, ptr_type data_address)
    : m_chunk_map(m_chunk_map)
    , m_base_address(data_address)
{
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

std::pair<jl::VM::InterpretResult, std::vector<jl::Operand>> jl::VM::run()
{
    patch_memmory_address(m_chunk_map, m_base_address);
    auto root_chunk = m_chunk_map.at("__root__");
    std::vector<Operand> temp_vars(root_chunk.get_max_allocated_temps());
    const auto result = run(root_chunk, temp_vars);

    return { result, temp_vars };
}

jl::VM::InterpretResult jl::VM::run(
    Chunk& chunk,
    std::vector<Operand>& temp_vars)
{
    const auto& irs = chunk.get_ir();
    const auto locations = fill_labels(irs, chunk.get_max_labels());
    uint32_t pc = 0;

    while (pc < irs.size()) {
        const auto& ir = irs[pc];
        if (debug_run)
            debug_print(chunk, pc, ir, temp_vars);
        pc = execute_ir(ir, pc, chunk, temp_vars, locations);
    }

    return InterpretResult::OK;
}

uint32_t jl::VM::execute_ir(
    Ir ir,
    uint32_t pc,
    Chunk& chunk,
    std::vector<Operand>& temp_vars,
    const std::vector<uint32_t> locations)
{
    if (ir.opcode() == OpCode::CALL) {
        const auto& cir = ir.call();
        auto& func_chunk = m_chunk_map.at(cir.func_name);
        temp_vars[cir.return_var.idx] = run_function(cir, func_chunk, temp_vars);
        return pc + 1;
    }

    switch (ir.type()) {
    case Ir::BINARY:
        handle_binary_ir(ir, temp_vars);
        return pc + 1;
    case Ir::UNARY:
        handle_unary_ir(ir, temp_vars);
        return pc + 1;
    case Ir::JUMP_STORE:
    case Ir::CONTROL:
        return handle_control_ir(pc, ir, temp_vars, locations);
    case Ir::TYPE_CAST:
        handle_type_cast(ir, temp_vars);
        break;
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
    case jl::OpCode::BIT_NOT: {
        result = ~std::get<int>(operand);
    } break;
    case jl::OpCode::LOAD: {
        const auto& op = get_nested_data(operand, temp_vars);
        const auto offset = std::get<PtrVar>(op).offset;

        switch (get_type(op)) {
        case OperandType::CHAR_PTR: {
            const auto data = VM::read_data<char>(offset);
            result = Operand { data };
        } break;
        case OperandType::INT_PTR: {
            const auto data = VM::read_data<int_type>(offset);
            result = Operand { data };
        } break;
        case OperandType::FLOAT_PTR: {
            const auto data = VM::read_data<float_type>(offset);
            result = Operand { data };
        } break;
        case OperandType::BOOL_PTR: {
            const auto data = VM::read_data<bool>(offset);
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
            VM::set_data(std::get<PtrVar>(addr).offset, std::get<char>(data));
            break;
        case OperandType::INT_PTR:
            VM::set_data(std::get<PtrVar>(addr).offset, std::get<int_type>(data));
            break;
        case OperandType::FLOAT_PTR:
            VM::set_data(std::get<PtrVar>(addr).offset, std::get<float_type>(data));
            break;
        case OperandType::BOOL_PTR:
            VM::set_data(std::get<PtrVar>(addr).offset, std::get<bool>(data));
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

std::pair<jl::VM::InterpretResult, std::vector<jl::Operand>> jl::VM::interactive_execute()
{
    debug_run = true;
    patch_memmory_address(m_chunk_map, m_base_address);
    auto root_chunk = m_chunk_map.at("__root__");
    std::vector<Operand> temp_vars(root_chunk.get_max_allocated_temps());
    const auto result = run(root_chunk, temp_vars);
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
    Chunk& func_chunk,
    const std::vector<Operand>& temp_vars)
{
    if (!func_chunk.extern_symbol) {
        std::vector<Operand> stack_vars { func_chunk.get_max_allocated_temps() };
        for (int i = 0; i < ir.args.size(); i++) {
            // First temp var will always be the fucntion itself
            stack_vars[i + 1] = temp_vars[ir.args[i].idx];
        }

        run(func_chunk, stack_vars);
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
            func_chunk.return_type);

        return ret_value;
    }
}

template <typename T>
T convert_variant(const jl::Operand& v)
{
    return std::visit([](auto&& arg) -> T {
        if constexpr (std::is_convertible_v<decltype(arg), T>) {
            return static_cast<T>(arg);
        } else {
            throw std::runtime_error("Conversion not supported for this type.");
        }
    },
        v);
}

void jl::VM::handle_type_cast(const Ir& ir, std::vector<Operand>& temp_vars)
{
    if (ir.opcode() != OpCode::TYPE_CAST) {
        unimplemented();
    }

    const auto cir = ir.cast();
    Operand& from = temp_vars[cir.source.idx];
    Operand& to = temp_vars[cir.dest.idx];

    switch (cir.to) {
    case OperandType::INT:
        to = convert_variant<int_type>(from);
        break;
    case OperandType::FLOAT:
        to = convert_variant<float_type>(from);
        break;
    case OperandType::BOOL:
        to = convert_variant<bool>(from);
        break;
    case OperandType::CHAR:
        to = convert_variant<char>(from);
        break;
    case OperandType::CHAR_PTR:
    case OperandType::INT_PTR:
    case OperandType::FLOAT_PTR:
    case OperandType::BOOL_PTR:
    case OperandType::NIL_PTR:
        // No need for type casting, just return
        // TODO: Avoid writing this cast during code generation itself
        to = temp_vars[cir.source.idx];
        std::get<PtrVar>(to).type = cir.to;
        return;
    case OperandType::TEMP:
    case OperandType::UNASSIGNED:
    case OperandType::NIL:
        unimplemented();
    }
}

void jl::VM::patch_memmory_address(std::map<std::string, Chunk>& m_chunk_map, uint64_t base_address)
{
    // Add the base address to all operands in ir which is a pointer type
    for (auto& [name, chunk] : m_chunk_map) {
        for (auto& ir : chunk.get_ir()) {
            const auto type = ir.type();
            if (type == Ir::UNARY && is_pure_ptr(get_type(ir.unary().operand))) {
                std::get<PtrVar>(std::get<UnaryIr>(ir.data).operand).offset += base_address;
            } else if (type == Ir::JUMP_STORE && is_pure_ptr(get_type(ir.jump().target))) {
                std::get<PtrVar>(std::get<JumpStoreIr>(ir.data).target).offset += base_address;
            } else if (type == Ir::CONTROL && is_pure_ptr(get_type(ir.control().data))) {
                std::get<PtrVar>(std::get<ControlIr>(ir.data).data).offset += base_address;
            }
        }
    }
}