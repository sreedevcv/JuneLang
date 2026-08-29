#pragma once

#include "BasicBlock.hpp"
#include "Function.hpp"
#include "codegen/x86/MachineBlock.hpp"
#include "codegen/x86/MachineFunction.hpp"
#include "ir/IR.hpp"
#include "ir/IRVisitor.hpp"
#include "value/Variable.hpp"

namespace jl {
namespace x86 {
    class Generator : public ir::IRVisitor {
    public:
        Generator(Function* function);

        ~Generator() = default;

        MachineFunction generate();

        void generate(BasicBlock* function);

    private:
        Function* m_function;
        MachineFunction m_out;
        MachineBlock* m_curr_block;
        MachineBlock* m_epilogue_block;

        void set_current_block(MachineBlock* block);

        uint32_t get_stack_offset(value::Variable var);

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

        void visit_type_cast_ir(ir::TypeCast& type_cast) override;

        void visit_phi(ir::Phi& phi) override;
    };
}
}
