#pragma once

#include "Function.hpp"
#include "Module.hpp"
#include "ir/Binary.hpp"
#include "ir/IRVisitor.hpp"
#include "types/Type.hpp"

#include <array>
#include <cstdint>
#include <functional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>

namespace jl {
class x86CodeGen : ir::IRVisitor {
public:
    x86CodeGen(Module& module);

    std::stringstream generate();

private:
    Module& m_mod;
    std::stringstream m_out;
    std::stringstream m_data_section_out;

    void generate(const std::string& func_name, Function* function);

    struct VarInfo {
        uint32_t size;
        uint32_t offset;
        const type::Type* type;
    };

    VarInfo get_var_info(uint32_t id) const;

    std::pair<uint32_t, uint32_t> get_size_and_offset(uint32_t id) const;

    using lambda_t = std::function<std::string(uint32_t, uint32_t, uint32_t)>;

    void move_data(
        lambda_t& mov_lambda,
        uint32_t& total_size,
        uint32_t& offset1,
        uint32_t& offset2);

    void do_sized_move(
        lambda_t& mov_lambda,
        uint32_t& total_size,
        uint32_t fixed_size,
        uint32_t& offset1,
        uint32_t& offset2);

    void visit_binary_ir(ir::Binary& binary) override;

    void visit_move_ir(ir::Move& move) override;

    void visit_return_ir(ir::Return& ret) override;

    void visit_call_ir(ir::Call& call) override;

    void visit_jump_ir(ir::Jump& jump) override;

    void visit_cond_jump_ir(ir::CondJump& jump) override;

    void visit_unary_ir(ir::Unary& unary) override;

    void visit_label_ir(ir::Label& label) override;

    void visit_allocate_list_ir(ir::AllocateList& allocate) override;

    void visit_allocate_var_ir(ir::AllocateVar& allocate) override;

    void visit_read_ir(ir::Read& read) override;

    void visit_write_ir(ir::Write& write) override;

    void visit_init_literal_ir(ir::InitLiteral& literal) override;

    void visit_debug_print_ir(ir::DebugPrint& print) override;

    const std::array<std::string, 6> m_arg_registers {
        "di",
        "si",
        "dx",
        "rcx",
        "r8",
        "r9",
    };

    const std::unordered_map<uint8_t, const char*> m_size_to_ptr_map {
        { 1, "BYTE" },
        { 2, "WORD" },
        { 4, "DWORD" },
        { 8, "QWORD" }
    };

    const std::unordered_map<ir::Binary::Operation, std::pair<const char*, const char*>> m_arith_opers {
        { ir::Binary::PLUS, { "add", "addsd" } },
        { ir::Binary::MINUS, { "sub", "subsd" } },
        { ir::Binary::STAR, { "imul", "mulsd" } },
        { ir::Binary::SLASH, { "idiv", "divsd" } },
        { ir::Binary::GREATER, { "cmp", "ucomisd" } },
        { ir::Binary::LESS, { "cmp", "ucomisd" } },
        { ir::Binary::GREATER_EQUAL, { "cmp", "ucomisd" } },
        { ir::Binary::LESS_EQUAL, { "cmp", "ucomisd" } },
        { ir::Binary::EQUAL_EQUAL, { "cmp", "ucomisd" } },
        { ir::Binary::BANG_EQUAL, { "cmp", "ucomisd" } },
    };

    const std::unordered_map<ir::Binary::Operation, std::pair<const char*, const char*>> m_cmp_opers {
        { ir::Binary::LESS, { "setl", "setb" } },
        { ir::Binary::LESS_EQUAL, { "setle", "setbe" } },
        { ir::Binary::GREATER, { "setg", "seta" } },
        { ir::Binary::GREATER_EQUAL, { "setge", "setae" } },
        { ir::Binary::EQUAL_EQUAL, { "sete", "sete" } },
        { ir::Binary::BANG_EQUAL, { "setne", "setne" } },
    };

    const Function* m_current_func;
    const std::string* m_current_func_name;
};
}
