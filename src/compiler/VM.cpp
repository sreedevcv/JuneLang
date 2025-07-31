#include "VM.hpp"

#include <functional>
#include <iostream>
#include <utility>

#include "Ir.hpp"
#include "OpCode.hpp"
#include "Operand.hpp"
#include "Utils.hpp"

template <typename FromType, typename ToType>
static void typecast(jl::reg_type& from, jl::reg_type& to)
{
    FromType fdata;
    ToType tdata;
    std::memcpy(&fdata, &from, sizeof(FromType));
    tdata = static_cast<ToType>(fdata);
    std::memcpy(&to, &tdata, sizeof(ToType));
}

jl::VM::VM(const std::map<std::string, Chunk>& m_chunk_map, ptr_type data_address)
    : m_chunk_map(m_chunk_map)
    , m_base_address(data_address)
{
    // Prepare the dispatch table
    const auto add_to_table = [&](OperandType t1, OperandType t2, casting_func_t f) {
        m_dispatch_table[{ t1, t2 }] = f;
    };

#define ADD_TO_TABLE(FROM, TO) \
    add_to_table(OperandType::FROM, OperandType::TO, typecast<PrimitiveType<OperandType::FROM>::type, PrimitiveType<OperandType::TO>::type>)

    ADD_TO_TABLE(INT, FLOAT);
    ADD_TO_TABLE(FLOAT, INT);

    ADD_TO_TABLE(INT, CHAR);
    ADD_TO_TABLE(CHAR, INT);

    ADD_TO_TABLE(NIL_PTR, CHAR_PTR);
    ADD_TO_TABLE(NIL_PTR, INT_PTR);
    ADD_TO_TABLE(NIL_PTR, FLOAT_PTR);
    ADD_TO_TABLE(NIL_PTR, BOOL_PTR);

    ADD_TO_TABLE(CHAR_PTR, NIL_PTR);
    ADD_TO_TABLE(INT_PTR, NIL_PTR);
    ADD_TO_TABLE(FLOAT_PTR, NIL_PTR);
    ADD_TO_TABLE(BOOL_PTR, NIL_PTR);
#undef ADD_TO_TABLE
}

template <typename T>
static jl::reg_type store_in_reg(T& data)
{
    jl::reg_type reg;
    std::memcpy(&reg, &data, sizeof(T));
    return reg;
}

static jl::reg_type extract_data(const jl::Operand& op)
{
    switch (jl::get_type(op)) {
    case jl::OperandType::TEMP:
    case jl::OperandType::UNASSIGNED:
        unimplemented();
    case jl::OperandType::NIL:
        return 0;
    case jl::OperandType::INT:
        return store_in_reg(std::get<jl::int_type>(op));
    case jl::OperandType::FLOAT:
        return store_in_reg(std::get<jl::float_type>(op));
    case jl::OperandType::BOOL:
        return store_in_reg(std::get<bool>(op));
    case jl::OperandType::CHAR:
        return store_in_reg(std::get<char>(op));
    case jl::OperandType::CHAR_PTR:
    case jl::OperandType::INT_PTR:
    case jl::OperandType::FLOAT_PTR:
    case jl::OperandType::BOOL_PTR:
    case jl::OperandType::NIL_PTR:
        return store_in_reg(std::get<jl::PtrVar>(op).offset);
    }
    unimplemented();
    return 0;
}

static jl::reg_type nested_extract(const jl::Operand& operand, const std::vector<jl::reg_type>& temp_vars)
{
    return jl::get_type(operand) == jl::OperandType::TEMP
        ? temp_vars[std::get<jl::TempVar>(operand).idx]
        : extract_data(operand);
}

template <typename Op>
static const jl::reg_type execute_bitwise_and_modulus(
    const jl::reg_type& op1,
    const jl::reg_type& op2,
    Op bin_oper)
{
    return bin_oper(op1, op2);
}

// TODO::Make sure both operands are int or float before arithametics
template <typename Op>
static const jl::reg_type execute_arithametic(
    const jl::reg_type& op1,
    const jl::reg_type& op2,
    bool is_float,
    Op bin_oper)
{
    if (is_float) {
        jl::float_type f1 = jl::VM::get<jl::float_type>(op1);
        jl::float_type f2 = jl::VM::get<jl::float_type>(op2);
        jl::reg_type reg;
        jl::float_type result = bin_oper(f1, f2);

        std::memcpy(&reg, &result, sizeof(jl::float_type));
        return reg;
    } else {
        return bin_oper(op1, op2);
    }
}

template <typename Op>
static const jl::reg_type execute_boolean(
    const jl::reg_type& op1,
    const jl::reg_type& op2,
    Op bin_oper)
{
    return bin_oper(op1, op2);
}

static const jl::reg_type do_arithametic(
    const jl::reg_type& op1,
    const jl::reg_type& op2,
    const jl::OperandType operation_type,
    jl::OpCode opcode)
{
    jl::reg_type result;
    bool is_float = false;

    if (operation_type == jl::OperandType::FLOAT) {
        is_float = true;
    }

    switch (opcode) {
    case jl::OpCode::ADD:
        result = execute_arithametic(op1, op2, is_float, std::plus<> {});
        break;
    case jl::OpCode::MINUS:
        result = execute_arithametic(op1, op2, is_float, std::minus<> {});
        break;
    case jl::OpCode::STAR:
        result = execute_arithametic(op1, op2, is_float, std::multiplies<> {});
        break;
    case jl::OpCode::SLASH:
        result = execute_arithametic(op1, op2, is_float, std::divides<> {});
        break;
    case jl::OpCode::GREATER:
        result = execute_arithametic(op1, op2, is_float, std::greater<> {});
        break;
    case jl::OpCode::LESS:
        result = execute_arithametic(op1, op2, is_float, std::less<> {});
        break;
    case jl::OpCode::GREATER_EQUAL:
        result = execute_arithametic(op1, op2, is_float, std::greater_equal<> {});
        break;
    case jl::OpCode::LESS_EQUAL:
        result = execute_arithametic(op1, op2, is_float, std::less_equal<> {});
        break;
    case jl::OpCode::EQUAL:
        result = execute_arithametic(op1, op2, is_float, std::equal_to<> {});
        break;
    case jl::OpCode::NOT_EQUAL:
        result = execute_arithametic(op1, op2, is_float, std::not_equal_to<> {});
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

std::pair<jl::VM::InterpretResult, std::vector<jl::reg_type>> jl::VM::run()
{
    auto root_chunk = m_chunk_map.at("__root__");
    std::vector<reg_type> temp_vars(root_chunk.get_max_allocated_temps());
    const auto result = run(root_chunk, temp_vars);

    return { result, temp_vars };
}

jl::VM::InterpretResult jl::VM::run(
    const Chunk& chunk,
    std::vector<reg_type>& temp_vars)
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
    const Chunk& chunk,
    std::vector<reg_type>& temp_vars,
    const std::vector<uint32_t> locations)
{
    if (ir.opcode() == OpCode::CALL) {
        const auto& cir = ir.call();
        auto& func_chunk = m_chunk_map.at(cir.func_name);
        temp_vars[cir.return_var.idx] = run_function(cir, chunk, func_chunk, temp_vars);
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
    case Ir::LOAD_STORE:
        handle_load_store(ir, temp_vars);
        break;
    default:
        unimplemented();
    }

    return pc + 1;
}

void jl::VM::handle_binary_ir(const Ir& ir, std::vector<reg_type>& temp_vars)
{
    reg_type result;
    const auto& binar_ir = ir.binary();

    const auto& left = temp_vars[binar_ir.op1.idx];
    const auto& right = temp_vars[binar_ir.op2.idx];

    switch (jl::get_category(binar_ir.opcode)) {
    case jl::OperatorCategory::ARITHAMETIC:
    case jl::OperatorCategory::COMPARISON:
    case jl::OperatorCategory::BOOLEAN:
    case OperatorCategory::BITWISE_AND_MODULUS:
        result = do_arithametic(left, right, binar_ir.type, binar_ir.opcode);
        break;
    case jl::OperatorCategory::OTHER:
        unimplemented();
        break;
    }

    temp_vars[ir.dest().idx] = result;
}

void jl::VM::handle_unary_ir(const Ir& ir, std::vector<reg_type>& temp_vars)
{
    reg_type result;
    const auto& unary_ir = ir.unary();
    const auto operand = nested_extract(unary_ir.operand, temp_vars);

    switch (ir.opcode()) {
    case OpCode::MOVE: {
        result = operand;
    } break;
    case jl::OpCode::NOT: {
        result = !(operand);
    } break;
    case jl::OpCode::MINUS: {
        // TODO::Remove this, no longer needed as codegen converts it to binary
        result = -1 * operand;
    } break;
    case jl::OpCode::BIT_NOT: {
        result = ~operand;
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
    const Ir& ir,
    std::vector<reg_type>& temp_vars,
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
        if (condition == false) {
            const auto label = std::get<int>(ir.jump().target);
            return label_locations[label];
        } else {
            return pc + 1;
        }
    } break;
    case OpCode::RETURN: {
        const auto& operand = ir.control().data;
        const auto& data = nested_extract(operand, temp_vars);
        ;
        m_stack.push(data);
        return UINT_MAX;
    } break;
    default:
        unimplemented();
    }

    return pc + 1;
}

uint64_t read_bytes_to_uint64_le(const void* ptr, size_t size)
{
    uint64_t temp = 0;
    std::memcpy(&temp, ptr, size); // copies up to 8 bytes into temp
    return temp;
}

void write_bytes_from_uint64_le(void* ptr, uint64_t val, size_t size)
{
    memcpy(ptr, &val, size);
}

void jl::VM::handle_load_store(const Ir& ir, std::vector<reg_type>& temp_vars)
{
    const auto ls_ir = ir.load_store();
    const auto addr = temp_vars[ls_ir.addr.idx];

    if (ls_ir.opcode == OpCode::LOAD) {
        temp_vars[ls_ir.reg.idx] = read_bytes_to_uint64_le((char*)(addr), ls_ir.size);
    } else {
        // This should be a store
        write_bytes_from_uint64_le((char*)(addr), temp_vars[ls_ir.reg.idx], ls_ir.size);
    }
}

std::pair<jl::VM::InterpretResult, std::vector<jl::reg_type>> jl::VM::interactive_execute()
{
    debug_run = true;
    auto root_chunk = m_chunk_map.at("__root__");
    std::vector<reg_type> temp_vars(root_chunk.get_max_allocated_temps());
    const auto result = run(root_chunk, temp_vars);
    return { result, temp_vars };
}

void jl::VM::debug_print(
    const Chunk& chunk,
    uint32_t pc,
    const Ir& ir,
    const std::vector<reg_type>& temp_vars)
{
    std::cout << "================================================================================\n";

    std::cout << pc << " >";
    chunk.print_ir(std::cout, ir);
    std::cout << '\n';

    for (int i = 0; i < temp_vars.size(); i++) {
        if (i % 8 == 0)
            std::cout << '\n';

        std::cout << i << ": [";
        const auto& op = temp_vars[i];
        std::cout << pretty_print(op, chunk.get_nested_type(TempVar { static_cast<uint32_t>(i) })) << "]\t";
    }
    std::cout << '\n';

    char enter;
    std::cin.get();
}

jl::reg_type jl::VM::run_function(
    const CallIr& ir,
    const Chunk& curr_chunk,
    const Chunk& func_chunk,
    const std::vector<reg_type>& temp_vars)
{
    if (!func_chunk.extern_symbol) {
        std::vector<reg_type> stack_vars(func_chunk.get_max_allocated_temps());
        for (int i = 0; i < ir.args.size(); i++) {
            // First temp var will always be the fucntion itself
            stack_vars[i + 1] = temp_vars[ir.args[i].idx];
        }

        run(func_chunk, stack_vars);
        const auto ret_value = m_stack.top();

        return ret_value;
    } else {
        std::vector<std::pair<reg_type, OperandType>> args;
        for (int i = 0; i < ir.args.size(); i++) {
            const auto type = curr_chunk.get_nested_type(TempVar { ir.args[i].idx });
            args.push_back({ temp_vars[ir.args[i].idx], type });
        }

        reg_type ret_value = m_ffi.call(
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

void jl::VM::handle_type_cast(const Ir& ir, std::vector<reg_type>& temp_vars)
{
    if (ir.opcode() != OpCode::TYPE_CAST) {
        unimplemented();
    }

    const auto cir = ir.cast();
    auto& from = temp_vars[cir.source.idx];
    auto& to = temp_vars[cir.dest.idx];

    if (cir.from == cir.to) {
        to = from;
    } else {
        if (m_dispatch_table.contains({ cir.from, cir.to })) {
            m_dispatch_table[{ cir.from, cir.to }](from, to);
        } else {
            unimplemented();
        }
    }
}
