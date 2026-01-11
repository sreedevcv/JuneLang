#pragma once

#include "FuncBlock.hpp"
#include "ir/IRVisitor.hpp"
#include "value/Variable.hpp"

#include <array>
#include <cstdint>
#include <functional>
#include <sstream>
#include <string>
#include <unordered_map>

namespace jl {
class x86CodeGen : ir::IRVisitor {
public:
    x86CodeGen(std::unordered_map<std::string, std::unique_ptr<FuncBlock::FuncData>> ir_data);

    std::stringstream generate();

private:
    std::unordered_map<std::string, std::unique_ptr<FuncBlock::FuncData>> m_ir_data;

    std::stringstream m_out;

    void generate(const std::string& func_name, const FuncBlock::FuncData& func_data);

    value::VarData::Data get_size_and_offset(uint32_t id) const;

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

    void visit_unary_ir(ir::Unary& unary) override;

    void visit_label_ir(ir::Label& label) override;

    void visit_allocate_ir(ir::Allocate& allocate) override;

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

    const FuncBlock::FuncData* m_current_func;
    const std::string* m_current_func_name;
};
}
