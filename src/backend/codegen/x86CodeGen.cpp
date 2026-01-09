#include "x86CodeGen.hpp"

#include <cassert>
#include <cstdint>
#include <format>
#include <utility>
#include <variant>

#include "LiteralValue.hpp"
#include "Utils.hpp"
#include "ir/Binary.hpp"
#include "ir/Call.hpp"
#include "ir/InitLiteral.hpp"
#include "ir/Move.hpp"
#include "ir/Return.hpp"

jl::x86CodeGen::x86CodeGen(std::unordered_map<std::string, std::unique_ptr<jl::FuncBlock::FuncData>> ir_data)
    : m_ir_data(std::move(ir_data))
{
}

std::stringstream jl::x86CodeGen::generate()
{
    m_out << "global _start\n\n";
    m_out << "section .text\n\n";

    for (const auto& [name, data] : m_ir_data) {
        generate(name, *data.get());
    }

    m_out << "\n_start:\n";
    m_out << "call __root__\n";
    m_out << "mov eax, 60\n";
    m_out << "mov rdi, 0\n";
    m_out << "syscall\n";

    return std::move(m_out);
}

jl::value::VarData::Data jl::x86CodeGen::get_size_and_offset(uint32_t id) const
{
    return m_current_func->var_data.get_offset_map().at(id);
}

void jl::x86CodeGen::generate(const std::string& func_name, const FuncBlock::FuncData& func_data)
{
    if (func_data.type.get()->m_param_types.size() > 6) {
        unimplemented("Functions with parameters > 6 not supported");
    }

    m_out << func_name << ": \n";
    m_out << "push rbp\n";
    m_out << "mov rbp, rsp\n";

    // Allocate space on the stack
    const auto stack_space = func_data.var_data.total_size() + (func_data.var_data.total_size() % 16);
    m_out << std::format("sub rsp, {}\n", stack_space);

    // Move all arguments to stack
    for (size_t i = 0; i < func_data.type.get()->m_param_types.size(); i++) {
        const auto [size, offset] = func_data.var_data.get_offset_map().at(i);

        m_out << std::format("mov {} [rbp-{}], {}\n", m_size_to_ptr_map.at(size), offset + size, m_arg_registers[i]);
    }

    // Generate op code for the irs
    m_current_func = &func_data;
    m_current_func_name = &func_name;

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
    const auto [size1, offset1] = get_size_and_offset(binary.m_operand_a->id());
    const auto [size2, offset2] = get_size_and_offset(binary.m_operand_b->id());
    const auto [size3, offset3] = get_size_and_offset(binary.m_dest->id());

    m_out << std::format("mov rax, {} [rbp-{}]\n", m_size_to_ptr_map.at(size1), offset1 + size1);
    m_out << std::format("mov rbx, {} [rbp-{}]\n", m_size_to_ptr_map.at(size2), offset2 + size2);

    switch (binary.m_operation) {
    case ir::Binary::PLUS:
        m_out << "add rax, rbx\n";
        break;
    case ir::Binary::MINUS:
    case ir::Binary::STAR:
    case ir::Binary::SLASH:
    case ir::Binary::GREATER:
    case ir::Binary::LESS:
    case ir::Binary::GREATER_EQUAL:
    case ir::Binary::LESS_EQUAL:
    case ir::Binary::EQUAL_EQUAL:
    case ir::Binary::BANG_EQUAL:
    case ir::Binary::PERCENT:
    case ir::Binary::BIT_AND:
    case ir::Binary::BIT_OR:
    case ir::Binary::BIT_XOR:
    case ir::Binary::LOG_AND:
    case ir::Binary::LOG_OR:
        unimplemented("binary instruction");
        break;
    }

    m_out << std::format("mov {} [rbp-{}], rax\n", m_size_to_ptr_map.at(size3), offset3 + size3);
}

void jl::x86CodeGen::visit_move_ir(ir::Move& move)
{
    const auto [size1, offset1] = get_size_and_offset(move.m_source->id());
    const auto [size2, offset2] = get_size_and_offset(move.m_dest->id());

    m_out << std::format("mov rax, {} [rbp-{}]\n", m_size_to_ptr_map.at(size1), offset1 + size1);
    m_out << std::format("mov {} [rbp-{}], rax\n", m_size_to_ptr_map.at(size2), offset2 + size2);
}

void jl::x86CodeGen::visit_return_ir(ir::Return& ret)
{
    if (ret.m_ret_val) {
        const auto [size, offset] = get_size_and_offset(ret.m_ret_val.value()->id());
        m_out << std::format("mov rax, {} [rbp-{}]\n", m_size_to_ptr_map.at(size), offset + size);
    }

    m_out << "jmp " << *m_current_func_name << "_end\n";
}

void jl::x86CodeGen::visit_call_ir(ir::Call& call)
{
    assert(call.m_args.size() < 6 && "Calls should have less than 6 args");

    for (uint32_t i = 0; i < call.m_args.size(); i++) {
        const auto [size, offset] = m_current_func->var_data.get_offset_map().at(call.m_args[i]->id());
        m_out << std::format("mov {}, {} [rbp-{}] \n", m_arg_registers[i], m_size_to_ptr_map.at(size), offset + size);
    }

    const auto [size, offset] = m_current_func->var_data.get_offset_map().at(call.m_dest->id());

    m_out << "call " << call.m_name << '\n';
    m_out << std::format("mov {} [rbp-{}], rax\n", m_size_to_ptr_map.at(size), offset + size);
}

struct PrintVisitor {
    void operator()(const jl::LiteralValue::int_type val) { }
    void operator()(const jl::LiteralValue::float_type val) { }
    void operator()(const jl::LiteralValue::bool_type val) { }
    void operator()(const jl::LiteralValue::char_type val) { }
};

void jl::x86CodeGen::visit_init_literal_ir(ir::InitLiteral& literal)
{
    const auto [size, offset] = m_current_func->var_data.get_offset_map().at(literal.m_dest->id());

    m_out << std::format("mov {} [rbp-{}], ", m_size_to_ptr_map.at(size), offset + size);

    std::visit([this](auto&& value) {
        m_out << value << "\n";
    },
        literal.m_source.get()->m_data);
}
