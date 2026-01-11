#include "x86CodeGen.hpp"

#include <cassert>
#include <cstdint>
#include <format>
#include <sstream>
#include <string>
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

jl::x86CodeGen::x86CodeGen(std::unordered_map<std::string, std::unique_ptr<jl::FuncBlock::FuncData>> ir_data)
    : m_ir_data(std::move(ir_data))
{
}

std::stringstream jl::x86CodeGen::generate()
{
    m_out << "extern printf\n\n";
    m_out << "global main\n\n";
    m_out << "section .data\n\n";
    m_out << "printf_int_str: db '%d', 0xA, 0\n";
    m_out << "printf_char_str: db '%c', 0xA, 0\n\n";

    m_out << "section .text\n\n";

    for (const auto& [name, data] : m_ir_data) {
        generate(name, *data.get());
    }

    m_out << "\nmain:\n";
    m_out << "call __root__\n";
    m_out << "mov eax, 0\n";
    m_out << "ret\n";

    return std::move(m_out);
}

std::string select_register(char reg, uint32_t size)
{
    switch (size) {
    case 1:
        return std::format("{}l", reg);
    case 2:
        return std::format("{}x", reg);
    case 4:
        return std::format("e{}x", reg);
    case 8:
        return std::format("r{}x", reg);
    }

    unimplemented("Invalid register size");
    return "";
}

jl::value::VarData::Data jl::x86CodeGen::get_size_and_offset(uint32_t id) const
{
    auto data = m_current_func->var_data.get_offset_map().at(id);
    data.offset += data.size;
    return data;
}

void jl::x86CodeGen::generate(const std::string& func_name, const FuncBlock::FuncData& func_data)
{
    if (func_data.type.get()->m_param_types.size() > 6) {
        unimplemented("Functions with parameters > 6 not supported");
    }

    m_current_func = &func_data;
    m_current_func_name = &func_name;

    m_out << func_name << ": \n";
    m_out << "push rbp\n";
    m_out << "mov rbp, rsp\n";

    // Allocate space on the stack
    const auto stack_space = func_data.var_data.total_size() + (16 - func_data.var_data.total_size() % 16);
    m_out << std::format("sub rsp, {}\n", stack_space);

    // Move all arguments to stack
    for (size_t i = 0; i < func_data.type.get()->m_param_types.size(); i++) {
        const auto [size, offset] = get_size_and_offset(i);

        m_out << std::format("mov {} [rbp-{}], {}\n", m_size_to_ptr_map.at(size), offset, m_arg_registers[i]);
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

void jl::x86CodeGen::visit_binary_ir(ir::Binary& binary)
{
    m_out << "; visit_binary_ir: " << binary.line() << '\n';

    const auto [size1, offset1] = get_size_and_offset(binary.m_operand_a.id());
    const auto [size2, offset2] = get_size_and_offset(binary.m_operand_b.id());
    const auto [size3, offset3] = get_size_and_offset(binary.m_dest.id());

    m_out << std::format("mov rax, {} [rbp-{}]\n", m_size_to_ptr_map.at(size1), offset1);

    switch (binary.m_operation) {
    case ir::Binary::PLUS:
        m_out << std::format("add rax, {} [rbp-{}]\n", m_size_to_ptr_map.at(size2), offset2);
        break;
    case ir::Binary::MINUS:
        m_out << std::format("sub rax, {} [rbp-{}]\n", m_size_to_ptr_map.at(size2), offset2);
        break;
    case ir::Binary::STAR:
        m_out << std::format("imul rax, {} [rbp-{}]\n", m_size_to_ptr_map.at(size2), offset2);
        break;
    case ir::Binary::SLASH:
        m_out << "cqo\n"; // convert quad word to octword (extends sign or rax into rdx:rax)
        m_out << std::format("idiv {} [rbp-{}]\n", m_size_to_ptr_map.at(size2), offset2);
        break;
    case ir::Binary::GREATER:
        m_out << std::format("cmp rax, {} [rbp-{}]\n", m_size_to_ptr_map.at(size2), offset2);
        m_out << "setg al\n";
        m_out << "movzx rax, al\n";
        break;
    case ir::Binary::LESS:
        m_out << std::format("cmp rax, {} [rbp-{}]\n", m_size_to_ptr_map.at(size2), offset2);
        m_out << "setl al\n";
        m_out << "movzx rax, al\n";
        break;
    case ir::Binary::GREATER_EQUAL:
        m_out << std::format("cmp rax, {} [rbp-{}]\n", m_size_to_ptr_map.at(size2), offset2);
        m_out << "setge al\n";
        m_out << "movzx rax, al\n";
        break;
    case ir::Binary::LESS_EQUAL:
        m_out << std::format("cmp rax, {} [rbp-{}]\n", m_size_to_ptr_map.at(size2), offset2);
        m_out << "setle al\n";
        m_out << "movzx rax, al\n";
        break;
    case ir::Binary::EQUAL_EQUAL:
        m_out << std::format("cmp rax, {} [rbp-{}]\n", m_size_to_ptr_map.at(size2), offset2);
        m_out << "sete al\n";
        m_out << "movzx rax, al\n";
        break;
    case ir::Binary::BANG_EQUAL:
        m_out << std::format("cmp rax, {} [rbp-{}]\n", m_size_to_ptr_map.at(size2), offset2);
        m_out << "setne al\n";
        m_out << "movzx rax, al\n";
        break;
    case ir::Binary::PERCENT:
    case ir::Binary::BIT_AND:
    case ir::Binary::BIT_OR:
    case ir::Binary::BIT_XOR:
    case ir::Binary::LOG_AND:
    case ir::Binary::LOG_OR:
        unimplemented("binary instruction");
        break;
    }

    const auto a_reg = select_register('a', size3); // for logical ops, size3 will be byte(bool)
    m_out << std::format("mov {} [rbp-{}], {}\n", m_size_to_ptr_map.at(size3), offset3, a_reg);
}

void jl::x86CodeGen::move_data(
    const std::string& src,
    const std::string& dest,
    uint32_t& total_size,
    uint32_t& offset1,
    uint32_t& offset2)
{
    static constexpr uint32_t sizes[] = { 8, 4, 2, 1 };

    for (int i = 0; i < 4 && total_size > 0; i++) {
        do_sized_move(src, dest, total_size, sizes[i], offset1, offset2);
    }
}

void jl::x86CodeGen::do_sized_move(
    const std::string& src,
    const std::string& dest,
    uint32_t& total_size,
    uint32_t fixed_size,
    uint32_t& offset1,
    uint32_t& offset2)
{
    const auto a_reg = select_register('a', fixed_size);
    while (total_size >= fixed_size) {
        m_out << std::format("mov {}, {} [rbp-{}]\n", a_reg, m_size_to_ptr_map.at(fixed_size), offset1);
        m_out << std::format("mov {} [rbp-{}], {}\n", m_size_to_ptr_map.at(fixed_size), offset2, a_reg);
        total_size -= fixed_size;
        offset1 -= fixed_size;
        offset2 -= fixed_size;
    }
}

void jl::x86CodeGen::visit_move_ir(ir::Move& move)
{
    m_out << "; visit_move_ir: " << move.line() << '\n';

    auto [size1, offset1] = get_size_and_offset(move.m_source.id());
    auto [size2, offset2] = get_size_and_offset(move.m_dest.id());

    std::string t;
    move_data(t, t, size1, offset1, offset2);

    // m_out << std::format("mov rax, {} [rbp-{}]\n", m_size_to_ptr_map.at(size1), offset1);
    // m_out << std::format("mov {} [rbp-{}], rax\n", m_size_to_ptr_map.at(size2), offset2);
}

void jl::x86CodeGen::visit_return_ir(ir::Return& ret)
{
    m_out << "; visit_return_ir: " << ret.line() << '\n';

    if (ret.m_ret_val) {
        const auto [size, offset] = get_size_and_offset(ret.m_ret_val.value().id());
        const auto a_reg = select_register('a', size);

        m_out << std::format("mov {}, {} [rbp-{}]\n", a_reg, m_size_to_ptr_map.at(size), offset);
    }

    m_out << "jmp " << *m_current_func_name << "_end\n";
}

void jl::x86CodeGen::visit_call_ir(ir::Call& call)
{
    m_out << "; visit_call_ir: " << call.line() << '\n';

    assert(call.m_args.size() < 6 && "Calls should have less than 6 args");

    for (uint32_t i = 0; i < call.m_args.size(); i++) {
        const auto [size, offset] = get_size_and_offset(call.m_args[i].id());
        m_out << std::format("mov {}, {} [rbp-{}] \n", m_arg_registers[i], m_size_to_ptr_map.at(size), offset);
    }

    const auto [size, offset] = get_size_and_offset(call.m_dest.id());

    m_out << "call " << call.m_name << '\n';

    // Move the result from a register to destination
    const auto a_reg = select_register('a', size);
    m_out << std::format("mov {} [rbp-{}], {}\n", m_size_to_ptr_map.at(size), offset, a_reg);
}

void jl::x86CodeGen::visit_init_literal_ir(ir::InitLiteral& literal)
{
    m_out << "; visit_init_literal_ir: " << literal.line() << '\n';

    const auto [size, offset] = get_size_and_offset(literal.m_dest.id());
    m_out << std::format("mov {} [rbp-{}], ", m_size_to_ptr_map.at(size), offset);

    std::visit([this](auto&& value) { m_out << (uint64_t)value << "\n"; },
        literal.m_source.get()->m_data);
}

void jl::x86CodeGen::visit_debug_print_ir(ir::DebugPrint& print)
{
    m_out << "; visit_debug_print_ir: " << print.line() << '\n';

    switch (print.m_primitive) {
    case type::Builtin::INT:
        m_out << "mov rdi, printf_int_str\n";
        break;
    case type::Builtin::BOOL:
        m_out << "mov rdi, printf_int_str\n";
        break;
    case type::Builtin::CHAR:
        m_out << "mov rdi, printf_char_str\n";
        break;
    case type::Builtin::FLOAT:
    case type::Builtin::VOID:
        unimplemented("printf strings for float and void");
        break;
    }

    const auto [size, offset] = get_size_and_offset(print.m_val.id());

    m_out << std::format("mov rsi, {} [rbp-{}]\n", m_size_to_ptr_map.at(size), offset);
    m_out << "mov eax, 0\n";
    m_out << "call printf\n";
}

void jl::x86CodeGen::visit_jump_ir(ir::Jump& jump)
{
    m_out << "; visit_jump_ir: " << jump.line() << '\n';

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
    m_out << "; visit_unary_ir: " << unary.line() << '\n';

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
    m_out << "; visit_allocate_ir: " << allocate.line() << '\n';

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
    m_out << "; visit_read_ir: " << read.line() << '\n';

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
    auto b_reg = select_register('b', read.m_size);
    m_out << std::format("mov {}, {} [rax]\n", b_reg, m_size_to_ptr_map.at(read.m_size));
    // Move rbx to destination
    b_reg = select_register('b', mov_size);
    m_out << std::format("mov {} [rbp-{}], {}\n", m_size_to_ptr_map.at(mov_size), mov_offset, b_reg);
}

void jl::x86CodeGen::visit_write_ir(ir::Write& write)
{
    m_out << "; visit_write_ir: " << write.line() << '\n';

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
    auto b_reg = select_register('b', src_size);
    m_out << std::format("mov {}, {} [rbp-{}]\n", b_reg, m_size_to_ptr_map.at(src_size), src_offset);
    // Move the read value to rbx
    b_reg = select_register('b', write.m_size);
    m_out << std::format("mov {} [rax], {}\n", m_size_to_ptr_map.at(write.m_size), b_reg);
}
