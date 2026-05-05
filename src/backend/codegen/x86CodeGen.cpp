#include "x86CodeGen.hpp"

#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <format>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>

#include "LiteralValue.hpp"
#include "Utils.hpp"
#include "ir/Allocate.hpp"
#include "ir/Binary.hpp"
#include "ir/Call.hpp"
#include "ir/DebugPrint.hpp"
#include "ir/InitLiteral.hpp"
#include "ir/Jump.hpp"
#include "ir/Move.hpp"
#include "ir/Read.hpp"
#include "ir/Return.hpp"
#include "ir/Unary.hpp"
#include "ir/Write.hpp"
#include "types/Type.hpp"

constexpr auto int_str = "printf_int_str";
constexpr auto char_str = "printf_char_str";
constexpr auto float_str = "printf_float_str";
constexpr auto int_list_str = "printf_int_list_str";
constexpr auto char_list_str = "printf_char_list_str";
constexpr auto float_list_str = "printf_float_list_str";

jl::x86CodeGen::x86CodeGen(std::unordered_map<std::string, std::unique_ptr<jl::FuncBlock::BasicBlock>> ir_data)
    : m_ir_data(std::move(ir_data))
{
}

std::stringstream jl::x86CodeGen::generate()
{
    m_out << "extern printf\n\n";
    m_out << "global main\n\n";

    m_data_section_out << "\nsection .data\n\n";
    m_data_section_out << int_str << ": db '%ld', 0xA, 0\n";
    m_data_section_out << char_str << ": db '%c', 0xA, 0\n\n";
    m_data_section_out << float_str << ": db '%lf', 0xA, 0\n\n";
    m_data_section_out << int_list_str << ": db '%ld ', 0\n\n";
    m_data_section_out << char_list_str << ": db '%c', 0\n\n";
    m_data_section_out << float_list_str << ": db '%lf ', 0\n\n";

    m_out << "section .text\n\n";

    for (const auto& [name, data] : m_ir_data) {
        generate(name, *data.get());
    }

    m_out << "\nmain:\n";
    m_out << "call __root__\n";
    m_out << "mov eax, 0\n";
    m_out << "ret\n";

    m_out << m_data_section_out.str();

    return std::move(m_out);
}

const char* select_register(const char* reg, uint32_t size)
{
    static const std::unordered_map<std::string, std::array<const char*, 4>> reg_size_map = {
        { "a", { "rax", "eax", "ax", "al" } },
        { "b", { "rbx", "ebx", "bx", "bl" } },
        { "c", { "rcx", "ecx", "cx", "cl" } },
        { "d", { "rdx", "edx", "dx", "dl" } },
        { "si", { "rsi", "esi", "si", "sil" } },
        { "di", { "rdi", "edi", "di", "dil" } },
        { "bp", { "rbp", "ebp", "bp", "bpl" } },
        { "sp", { "rsp", "esp", "sp", "spl" } },
        { "r8", { "r8", "r8d", "r8w", "r8b" } },
        { "r9", { "r9", "r9d", "r9w", "r9b" } },
        { "r10", { "r10", "r10d", "r10w", "r10b" } },
        { "r11", { "r11", "r11d", "r11w", "r11b" } },
        { "r12", { "r12", "r12d", "r12w", "r12b" } },
        { "r13", { "r13", "r13d", "r13w", "r13b" } },
        { "r14", { "r14", "r14d", "r14w", "r14b" } },
        { "r15", { "r15", "r15d", "r15w", "r15b" } },
    };

    return reg_size_map.at(reg)[3 - std::log2(size)];
}

void jl::x86CodeGen::move_data(
    lambda_t& mov_lambda,
    uint32_t& total_size,
    uint32_t& offset1,
    uint32_t& offset2)
{
    static constexpr uint32_t sizes[] = { 8, 4, 2, 1 };

    for (int i = 0; i < 4 && total_size > 0; i++) {
        do_sized_move(mov_lambda, total_size, sizes[i], offset1, offset2);
    }
}

void jl::x86CodeGen::do_sized_move(
    lambda_t& mov_lambda,
    uint32_t& total_size,
    uint32_t fixed_size,
    uint32_t& offset1,
    uint32_t& offset2)
{
    while (total_size >= fixed_size) {
        m_out << mov_lambda(fixed_size, offset1, offset2);
        total_size -= fixed_size;
        offset1 -= fixed_size;
        offset2 -= fixed_size;
    }
}

jl::x86CodeGen::VarInfo jl::x86CodeGen::get_var_info(uint32_t id) const
{
    const auto& data = m_current_func->var_data.get_offset_map().at(id);
    VarInfo info = {
        .size = data.size,
        .offset = data.offset + data.size,
        .type = data.m_type
    };

    return info;
}

std::pair<uint32_t, uint32_t> jl::x86CodeGen::get_size_and_offset(uint32_t id) const
{
    const auto [size, offset, _] = get_var_info(id);
    return { size, offset };
}

void jl::x86CodeGen::generate(const std::string& func_name, const FuncBlock::BasicBlock& func_data)
{
    if (func_data.type->m_param_types.size() > 6) {
        unimplemented("Functions with parameters > 6 not supported");
    }

    m_current_func = &func_data;
    m_current_func_name = &func_name;

    m_out << func_name << ": \n";
    m_out << "push rbp\n";
    m_out << "mov rbp, rsp\n";

    // Allocate space on the stack
    const auto stack_space = func_data.var_data.total_size() + (16 - func_data.var_data.total_size() % 16);
    m_out << std::format("sub rsp, {}\n", stack_space + 8); /* 8 is added to account for the  rbp that we are pushing. This will make stack 16byte aligned*/

    // Move all arguments to stack
    uint32_t arg_reg = 0;
    for (size_t i = 0; i < func_data.type->m_param_types.size(); i++) {
        const auto [size, offset] = get_size_and_offset(i);

        if (size > 8)
            continue;

        const auto sized_reg = select_register(m_arg_registers[arg_reg].c_str(), size);
        m_out << std::format("mov {} [rbp-{}], {}\n", m_size_to_ptr_map.at(size), offset, sized_reg);
        arg_reg += 1;
    }

    // Pop all the larges values into the stack
    for (uint32_t i = 0; i < func_data.type->m_param_types.size(); i++) {
        auto [size_src, offset_dest] = get_size_and_offset(i);
        if (size_src <= 8)
            continue;

        static lambda_t mov_lambda = [this](uint32_t size, uint32_t offset1, uint32_t offset2) {
            static const auto a_reg = select_register("a", size);
            return std::format("mov {}, {} [rbp+{}]\n", a_reg, m_size_to_ptr_map.at(size), -offset1)
                + std::format("mov {} [rbp-{}], {}\n", m_size_to_ptr_map.at(size), offset2, a_reg);
        };

        uint32_t stack_offset = -16;
        move_data(mov_lambda, size_src, stack_offset, offset_dest);
    }

    // Generate op code for the irs

    for (const auto& ir : func_data.irs) {
        ir->accept(*this);
    }

    m_out << *m_current_func_name << "_end: \n";
    m_out << "mov rsp, rbp\n";
    m_out << "pop rbp\n";
    m_out << "ret\n\n";
}

// --------------------------------------------------VISITOR-FUNCTIONS--------------------------------------------------

void jl::x86CodeGen::visit_binary_ir(ir::Binary& binary)
{
    m_out << "\n; visit_binary_ir: " << binary.line() << '\n';

    const auto [size1, offset1] = get_size_and_offset(binary.m_operand_a.id());
    const auto [size2, offset2] = get_size_and_offset(binary.m_operand_b.id());
    const auto [size3, offset3] = get_size_and_offset(binary.m_dest.id());
    const auto pair = m_arith_opers.at(binary.m_operation);
    const auto bin_op = binary.m_is_float ? pair.second : pair.first;
    auto mov_op = "mov";
    auto mov_reg = "rax";

    if (binary.m_is_float) {
        mov_op = "movsd";
        mov_reg = "xmm0";
    }

    m_out << std::format("{} {}, {} [rbp-{}]\n", mov_op, mov_reg, m_size_to_ptr_map.at(size1), offset1);

    switch (binary.m_operation) {
    case ir::Binary::PLUS:
    case ir::Binary::MINUS:
    case ir::Binary::STAR:
        m_out << std::format("{} {}, {} [rbp-{}]\n", bin_op, mov_reg, m_size_to_ptr_map.at(size2), offset2);
        break;
    case ir::Binary::SLASH:
        if (!binary.m_is_float) {
            m_out << "cqo\n"; // convert quad word to octword (extends sign or rax into rdx:rax)
            m_out << std::format("idiv {} [rbp-{}]\n", m_size_to_ptr_map.at(size2), offset2);
        } else {
            m_out << std::format("{} {}, {} [rbp-{}]\n", bin_op, mov_reg, m_size_to_ptr_map.at(size2), offset2);
        }
        break;
    case ir::Binary::GREATER:
    case ir::Binary::LESS:
    case ir::Binary::GREATER_EQUAL:
    case ir::Binary::LESS_EQUAL:
    case ir::Binary::EQUAL_EQUAL:
    case ir::Binary::BANG_EQUAL: {
        const auto pair = m_cmp_opers.at(binary.m_operation);
        const auto flag_oper = binary.m_is_float ? pair.second : pair.first;
        m_out << std::format("{} {}, {} [rbp-{}]\n", bin_op, mov_reg, m_size_to_ptr_map.at(size2), offset2);
        m_out << flag_oper << " al\n";
        m_out << "movzx rax, al\n";
    } break;
    case ir::Binary::PERCENT:
    case ir::Binary::BIT_AND:
    case ir::Binary::BIT_OR:
    case ir::Binary::BIT_XOR:
    case ir::Binary::LOG_AND:
    case ir::Binary::LOG_OR:
        unimplemented("binary instruction");
        break;
    }

    if (binary.m_is_float) {
        m_out << std::format("movsd {} [rbp-{}], xmm0\n", m_size_to_ptr_map.at(size3), offset3);
    } else {
        const auto a_reg = select_register("a", size3); // for logical ops, size3 will be byte(bool)
        m_out << std::format("mov {} [rbp-{}], {}\n", m_size_to_ptr_map.at(size3), offset3, a_reg);
    }
}

void jl::x86CodeGen::visit_move_ir(ir::Move& move)
{
    m_out << "\n; visit_move_ir: " << move.line() << '\n';

    auto [size1, offset1] = get_size_and_offset(move.m_source.id());
    auto [size2, offset2] = get_size_and_offset(move.m_dest.id());

    if (size1 <= 8) {
        const auto a_reg = select_register("a", size1);
        m_out << std::format("mov {}, {} [rbp-{}]\n", a_reg, m_size_to_ptr_map.at(size1), offset1);
        m_out << std::format("mov {} [rbp-{}], {}\n", m_size_to_ptr_map.at(size1), offset2, a_reg);
        return;
    }

    static lambda_t mov_lambda = [this](uint32_t size, uint32_t offset1, uint32_t offset2) -> std::string {
        const auto a_reg = select_register("a", size);
        return std::format("mov {}, {} [rbp-{}]\n", a_reg, m_size_to_ptr_map.at(size), offset1)
            + std::format("mov {} [rbp-{}], {}\n", m_size_to_ptr_map.at(size), offset2, a_reg);
    };

    move_data(mov_lambda, size1, offset1, offset2);
}

void jl::x86CodeGen::visit_return_ir(ir::Return& ret)
{
    m_out << "\n; visit_return_ir: " << ret.line() << '\n';

    if (ret.m_ret_val) {
        const auto [size, offset] = get_size_and_offset(ret.m_ret_val.value().id());
        const auto a_reg = select_register("a", size);
        m_out << std::format("mov {}, {} [rbp-{}]\n", a_reg, m_size_to_ptr_map.at(size), offset);
    }

    m_out << "jmp " << *m_current_func_name << "_end\n";
}

void jl::x86CodeGen::visit_call_ir(ir::Call& call)
{
    m_out << "\n; visit_call_ir: " << call.line() << '\n';

    assert(call.m_args.size() < 6 && "Calls should have less than 6 args");

    uint32_t arg_reg = 0;
    uint32_t stack_alloc_size = 0;
    for (uint32_t i = 0; i < call.m_args.size(); i++) {
        const auto [size, offset] = get_size_and_offset(call.m_args[i].id());
        if (size <= 8) {
            const auto sized_reg = select_register(m_arg_registers[arg_reg].c_str(), size);
            m_out << std::format("mov {}, {} [rbp-{}] \n", sized_reg, m_size_to_ptr_map.at(size), offset);
            arg_reg += 1;
        } else {
            stack_alloc_size += size;
        }
    }

    if (stack_alloc_size > 0) {
        m_out << "sub rsp, " << stack_alloc_size << "\n";
    }

    // Push all the larges values into the stack
    for (uint32_t i = 0; i < call.m_args.size(); i++) {
        auto [size_src, offset_src] = get_size_and_offset(call.m_args[i].id());
        if (size_src <= 8)
            continue;

        static lambda_t mov_lambda = [this](uint32_t size, uint32_t offset1, uint32_t offset2) {
            static const auto a_reg = select_register("a", size);
            return std::format("mov {}, {} [rbp-{}]\n", a_reg, m_size_to_ptr_map.at(size), offset1)
                + std::format("mov {} [rsp+{}], {}\n", m_size_to_ptr_map.at(size), -offset2, a_reg);
        };

        uint32_t stack_offset = 0;
        move_data(mov_lambda, size_src, offset_src, stack_offset);
    }

    m_out << "call " << call.m_name << '\n';

    // Move the result from a register to destination
    const auto [size, offset] = get_size_and_offset(call.m_dest.id());
    if (size > 0) {
        // For non void return types
        const auto a_reg = select_register("a", size);
        m_out << std::format("mov {} [rbp-{}], {}\n", m_size_to_ptr_map.at(size), offset, a_reg);
    }
}

void jl::x86CodeGen::visit_init_literal_ir(ir::InitLiteral& literal)
{
    m_out << "\n; visit_init_literal_ir: " << literal.line() << '\n';

    std::visit([this, &literal](auto&& value) {
        const auto [size, offset] = get_size_and_offset(literal.m_dest.id());
        using T = std::decay_t<decltype(value)>;

        if constexpr (std::is_same_v<T, LiteralValue::int_type>) {
            m_out << std::format("mov {} [rbp-{}], ", m_size_to_ptr_map.at(size), offset);
            m_out << value << "\n";
        } else if constexpr (std::is_same_v<T, LiteralValue::char_type>) {
            m_out << std::format("mov {} [rbp-{}], ", m_size_to_ptr_map.at(size), offset);
            m_out << std::format("{:d}", value) << "\n";
        } else if constexpr (std::is_same_v<T, LiteralValue::bool_type>) {
            m_out << std::format("mov {} [rbp-{}], ", m_size_to_ptr_map.at(size), offset);
            m_out << value << "\n";
        } else if constexpr (std::is_same_v<T, LiteralValue::float_type>) {
            const auto float_val = std::get<LiteralValue::float_type>(literal.m_source.get()->m_data);
            const auto tag = std::format("float_{}_{:f}", offset, float_val);
            m_data_section_out << std::format("{}: dq {:f}\n", tag, float_val);
            m_out << std::format("movsd xmm0, [rel {}]\n", tag);
            m_out << std::format("movsd {} [rbp-{}], xmm0\n", m_size_to_ptr_map.at(size), offset);
            m_out << '\n';
        } else {
            unimplemented("Type of literal value is not found");
        }
    },
        literal.m_source.get()->m_data);
}

void jl::x86CodeGen::visit_debug_print_ir(ir::DebugPrint& print)
{
    m_out << "\n; visit_debug_print_ir: " << print.line() << '\n';

    const auto [size, offset, type] = get_var_info(print.m_val.id());

    if (!print.m_is_list) {

        switch (print.m_primitive) {
        case type::Builtin::INT:
            m_out << "mov rdi," << int_str << '\n';
            break;
        case type::Builtin::BOOL:
            m_out << "mov rdi," << int_str << '\n';
            break;
        case type::Builtin::CHAR:
            m_out << "mov rdi," << char_str << '\n';
            break;
        case type::Builtin::FLOAT:
            m_out << "mov rdi," << float_str << '\n';
            break;
        case type::Builtin::VOID:
            unimplemented("printf strings for float and void");
            break;
        }

        const auto is_float = dynamic_cast<const type::Builtin*>(type)->m_primitive == type::Builtin::FLOAT;
        if (is_float) {
            m_out << std::format("movsd xmm0, {} [rbp-{}]\n", m_size_to_ptr_map.at(size), offset);
            m_out << "mov eax, 1\n";
        } else {
            const auto si_reg = select_register("si", size);
            const auto mov_op = size == 8 ? "mov" : "movsx";
            m_out << std::format("{} rsi, {} [rbp-{}]\n", mov_op, m_size_to_ptr_map.at(size), offset);
            m_out << "mov eax, 0\n";
        }
        m_out << "call printf\n";
    } else {
        const auto loop_start_label = std::format("printloopstart_{}_{}", *m_current_func_name, print.m_line);
        const auto loop_end_label = std::format("printloopend_{}_{}", *m_current_func_name, print.m_line);
        // move list base address to rax
        m_out << std::format("mov rax, {} [rbp-{}]\n", m_size_to_ptr_map.at(8), offset);
        // move list size to rcx
        m_out << std::format("mov rcx, {} [rbp-{}]\n", m_size_to_ptr_map.at(8), offset - 8);
        // add a label to loop around
        m_out << loop_start_label << ":\n";
        // Break out of the loop
        m_out << "cmp rcx, 0\n";
        m_out << "jle " << loop_end_label << '\n';
        // push rax and rcx to save it
        m_out << "push rax\n";
        m_out << "push rcx\n";
        // print content at base address
        switch (print.m_primitive) {
        case type::Builtin::INT:
            m_out << "mov rdi," << int_list_str << '\n';
            break;
        case type::Builtin::BOOL:
            m_out << "mov rdi," << int_list_str << '\n';
            break;
        case type::Builtin::CHAR:
            m_out << "mov rdi," << char_list_str << '\n';
            break;
        case type::Builtin::FLOAT:
            m_out << "mov rdi," << float_list_str << '\n';
            break;
        case type::Builtin::VOID:
            unimplemented("printf strings for float and void");
            break;
        }

        if (print.m_primitive != type::Builtin::FLOAT) {
            const auto si_reg = select_register("si", 8);
            m_out << std::format("mov {}, {} [rax]\n", si_reg, m_size_to_ptr_map.at(8));
            m_out << "mov eax, 0\n";
        } else {
            m_out << std::format("movsd xmm0, QWORD [rax]\n", offset);
            m_out << "mov eax, 1\n";
        }
        m_out << "call printf\n";
        // retrieve rax and rcx
        m_out << "pop rcx\n";
        m_out << "pop rax\n";
        // decrease base address
        m_out << std::format("add rax, {}\n", print.m_list_elem_size);
        // decrease rcx
        m_out << "dec rcx\n";
        // loop back
        m_out << "jmp " << loop_start_label << '\n';
        // add a label to skip loop
        m_out << loop_end_label << ":\n";
    }
}

void jl::x86CodeGen::visit_jump_ir(ir::Jump& jump)
{
    m_out << "\n; visit_jump_ir: " << jump.line() << '\n';

    if (jump.m_condition) {
        const auto [size, offset] = get_size_and_offset(jump.m_condition.value().id());
        m_out << std::format("mov bl, {} [rbp-{}]\n", m_size_to_ptr_map.at(size), offset);
        m_out << "cmp bl, 1\n";
        m_out << "jne label_" << *m_current_func_name << '_' << jump.m_label << '\n';
    } else {
        m_out << "jmp label_" << *m_current_func_name << '_' << jump.m_label << '\n';
    }
}

void jl::x86CodeGen::visit_unary_ir(ir::Unary& unary)
{
    m_out << "\n; visit_unary_ir: " << unary.line() << '\n';

    const auto [size1, offset1] = get_size_and_offset(unary.m_operand.id());
    m_out << std::format("mov rax, {} [rbp-{}]\n", m_size_to_ptr_map.at(size1), offset1);

    switch (unary.m_operation) {
    case ir::Unary::MINUS:
        m_out << "neg rax\n";
        break;
    case ir::Unary::BANG:
        m_out << "xor rax, 0x1\n";
        break;
    case ir::Unary::BIT_NOT:
        m_out << "not rax\n";
        break;
    }

    const auto [size2, offset2] = get_size_and_offset(unary.m_dest.id());
    m_out << std::format("mov {} [rbp-{}], rax\n", m_size_to_ptr_map.at(size2), offset2);
}

void jl::x86CodeGen::visit_label_ir(ir::Label& label)
{
    m_out << "label_" << *m_current_func_name << '_' << label.m_value << ":\n";
}

void jl::x86CodeGen::visit_allocate_ir(ir::Allocate& allocate)
{
    m_out << "\n; visit_allocate_ir: " << allocate.line() << '\n';

    const auto [s1, offset1] = get_size_and_offset(allocate.m_fat_ptr.id());
    const auto [s2, offset2] = get_size_and_offset(allocate.m_list.id());

    for (int i = 0; i < allocate.m_data.size(); i++) {
        const auto byte = allocate.m_data[i];
        m_out << std::format("mov {} [rbp-{}], {}\n", m_size_to_ptr_map.at(1), offset2 - i, byte);
    }

    // move the offset address of the list to first qword of list ptr
    m_out << std::format("lea rax, [rbp-{}]\n", offset2);
    m_out << std::format("mov {} [rbp-{}], rax\n", m_size_to_ptr_map.at(8), offset1);
    // move the element count of the list to the second qword of the list ptr
    m_out << std::format("mov {} [rbp-{}], {}\n", m_size_to_ptr_map.at(8), offset1 - 8, allocate.m_elem_count);
}

void jl::x86CodeGen::visit_read_ir(ir::Read& read)
{
    m_out << "\n; visit_read_ir: " << read.line() << '\n';

    const auto [base_size, base_offset] = get_size_and_offset(read.m_base.id());
    const auto [offset_size, real_offset] = get_size_and_offset(read.m_offset.id());
    const auto [mov_size, mov_offset] = get_size_and_offset(read.m_dest.id());

    // Load the offset
    m_out << std::format("mov rax, {} [rbp-{}]\n", m_size_to_ptr_map.at(offset_size), real_offset);
    // Multiply with offset multiplier
    m_out << std::format("imul rax, {}\n", read.m_offset_multiplier);
    // Add with the base address
    m_out << std::format("add rax, {} [rbp-{}]\n", m_size_to_ptr_map.at(8), base_offset);
    // Move the read value to rbx
    auto b_reg = select_register("b", read.m_size);
    m_out << std::format("mov {}, {} [rax]\n", b_reg, m_size_to_ptr_map.at(read.m_size));
    // Move rbx to destination
    b_reg = select_register("b", mov_size);
    m_out << std::format("mov {} [rbp-{}], {}\n", m_size_to_ptr_map.at(mov_size), mov_offset, b_reg);
}

void jl::x86CodeGen::visit_write_ir(ir::Write& write)
{
    m_out << "\n; visit_write_ir: " << write.line() << '\n';

    const auto [base_size, base_offset] = get_size_and_offset(write.m_base.id());
    const auto [offset_size, real_offset] = get_size_and_offset(write.m_offset.id());
    const auto [src_size, src_offset] = get_size_and_offset(write.m_src.id());

    // Load the offset
    m_out << std::format("mov rax, {} [rbp-{}]\n", m_size_to_ptr_map.at(offset_size), real_offset);
    // Multiply with offset multiplier
    m_out << std::format("imul rax, {}\n", write.m_offset_multiplier);
    // Add with the base address
    m_out << std::format("add rax, {} [rbp-{}]\n", m_size_to_ptr_map.at(8), base_offset);
    // Move src to b register
    auto b_reg = select_register("b", src_size);
    m_out << std::format("mov {}, {} [rbp-{}]\n", b_reg, m_size_to_ptr_map.at(src_size), src_offset);
    // Move the read value to rbx
    b_reg = select_register("b", write.m_size);
    m_out << std::format("mov {} [rax], {}\n", m_size_to_ptr_map.at(write.m_size), b_reg);
}
