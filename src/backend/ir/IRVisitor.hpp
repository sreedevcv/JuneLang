#pragma once

namespace jl {
namespace ir {
    class Binary;
    class Move;
    class Return;
    class Call;
    class InitLiteral;
    class DebugPrint;
    class Jump;
    class CondJump;
    class Unary;
    class Label;
    class AllocateList;
    class AllocateVar;
    class Read;
    class Write;
    class TypeCast;
    class Phi;

    struct IRVisitor {
        virtual void visit_binary_ir(ir::Binary& binary) = 0;

        virtual void visit_move_ir(ir::Move& move) = 0;

        virtual void visit_return_ir(ir::Return& ret) = 0;

        virtual void visit_call_ir(ir::Call& call) = 0;

        virtual void visit_jump_ir(ir::Jump& jump) = 0;

        virtual void visit_cond_jump_ir(ir::CondJump& jump) = 0;

        virtual void visit_unary_ir(ir::Unary& unary) = 0;

        virtual void visit_label_ir(ir::Label& label) = 0;

        virtual void visit_allocate_list_ir(ir::AllocateList& allocate) = 0;

        virtual void visit_allocate_var_ir(ir::AllocateVar& allocate) = 0;

        virtual void visit_read_ir(ir::Read& read) = 0;

        virtual void visit_write_ir(ir::Write& write) = 0;

        virtual void visit_init_literal_ir(ir::InitLiteral& literal) = 0;

        virtual void visit_debug_print_ir(ir::DebugPrint& print) = 0;

        virtual void visit_type_cast_ir(ir::TypeCast& type_cast) = 0;

        virtual void visit_phi(ir::Phi& phi) = 0;
    };
}

}
