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
    class Unary;
    class Label;

    struct IRVisitor {
        virtual void visit_binary_ir(ir::Binary& binary) = 0;

        virtual void visit_move_ir(ir::Move& move) = 0;

        virtual void visit_return_ir(ir::Return& ret) = 0;

        virtual void visit_call_ir(ir::Call& call) = 0;

        virtual void visit_jump_ir(ir::Jump& jump) = 0;

        virtual void visit_unary_ir(ir::Unary& unary) = 0;

        virtual void visit_label_ir(ir::Label& label) = 0;

        virtual void visit_init_literal_ir(ir::InitLiteral& literal) = 0;

        virtual void visit_debug_print_ir(ir::DebugPrint& print) = 0;
    };
}

}
