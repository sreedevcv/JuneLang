#pragma once

#include "Expr.hpp"
#include "codegen/x86/MachineBlock.hpp"
#include "codegen/x86/Register.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace jl {
namespace x86 {

    struct InstructionVisitor;
    struct Mov;
    struct Add;
    struct Sub;
    struct Less;
    struct LessEqual;
    struct Greater;
    struct GreaterEqual;
    struct NotEquals;
    struct Equals;
    struct Return;
    struct Push;
    struct Pop;
    struct Jump;
    struct JumpEqual;
    struct Cmp;
    struct Lea;
    struct Binary;

    struct Instruction {
        int32_t m_id = 0;

        virtual ~Instruction() = default;

        virtual std::string to_str() const = 0;

        virtual std::vector<VirtualRegister> defs() const = 0;

        virtual std::vector<VirtualRegister> uses() const = 0;

        virtual void accept(InstructionVisitor& visitor) = 0;
    };

    struct InstructionVisitor {
        virtual ~InstructionVisitor() = default;

        virtual void visit(Mov& inst) = 0;
        virtual void visit(Add& inst) = 0;
        virtual void visit(Sub& inst) = 0;
        virtual void visit(Less& inst) = 0;
        virtual void visit(LessEqual& inst) = 0;
        virtual void visit(Greater& inst) = 0;
        virtual void visit(GreaterEqual& inst) = 0;
        virtual void visit(Equals& inst) = 0;
        virtual void visit(NotEquals& inst) = 0;
        virtual void visit(Return& inst) = 0;
        virtual void visit(Push& inst) = 0;
        virtual void visit(Pop& inst) = 0;
        virtual void visit(Jump& inst) = 0;
        virtual void visit(JumpEqual& inst) = 0;
        virtual void visit(Cmp& inst) = 0;
        virtual void visit(Lea& inst) = 0;
    };

    struct Binary {
        VirtualRegister source;
        VirtualRegister dest;
    };

    struct Mov : public Instruction {
        VirtualRegister source;
        VirtualRegister dest;
        bool is_float;
        std::optional<SizeDirective> size;

        ~Mov() = default;

        std::string to_str() const override;

        std::vector<VirtualRegister> defs() const override;

        std::vector<VirtualRegister> uses() const override;

        void accept(InstructionVisitor& visitor) override;
    };

    struct Add : public Instruction {
        VirtualRegister source;
        VirtualRegister dest;
        bool is_float;

        ~Add() = default;

        std::string to_str() const override;

        std::vector<VirtualRegister> defs() const override;

        std::vector<VirtualRegister> uses() const override;

        void accept(InstructionVisitor& visitor) override;
    };

    struct Sub : public Instruction {
        VirtualRegister source;
        VirtualRegister dest;
        bool is_float;

        ~Sub() = default;

        std::string to_str() const override;

        std::vector<VirtualRegister> defs() const override;

        std::vector<VirtualRegister> uses() const override;

        void accept(InstructionVisitor& visitor) override;
    };

    struct Less : public Instruction {
        VirtualRegister reg;
        bool is_float;

        ~Less() = default;

        std::string to_str() const override;

        std::vector<VirtualRegister> defs() const override;

        std::vector<VirtualRegister> uses() const override;

        void accept(InstructionVisitor& visitor) override;
    };

    struct Equals : public Instruction {
        VirtualRegister reg;
        bool is_float;

        ~Equals() = default;

        std::string to_str() const override;

        std::vector<VirtualRegister> defs() const override;

        std::vector<VirtualRegister> uses() const override;

        void accept(InstructionVisitor& visitor) override;
    };

    struct Return : public Instruction {
        ~Return() = default;

        std::string to_str() const override;

        std::vector<VirtualRegister> defs() const override;

        std::vector<VirtualRegister> uses() const override;

        void accept(InstructionVisitor& visitor) override;
    };

    struct Push : public Instruction {
        VirtualRegister value;

        ~Push() = default;

        std::string to_str() const override;

        std::vector<VirtualRegister> defs() const override;

        std::vector<VirtualRegister> uses() const override;

        void accept(InstructionVisitor& visitor) override;
    };

    struct Pop : public Instruction {
        VirtualRegister value;

        ~Pop() = default;

        std::string to_str() const override;

        std::vector<VirtualRegister> defs() const override;

        std::vector<VirtualRegister> uses() const override;

        void accept(InstructionVisitor& visitor) override;
    };

    struct Jump : public Instruction {
        MachineBlock* target;

        ~Jump() = default;

        std::string to_str() const override;

        std::vector<VirtualRegister> defs() const override;

        std::vector<VirtualRegister> uses() const override;

        void accept(InstructionVisitor& visitor) override;
    };

    struct JumpEqual : public Jump {
        ~JumpEqual() = default;

        std::string to_str() const override;

        std::vector<VirtualRegister> defs() const override;

        std::vector<VirtualRegister> uses() const override;

        void accept(InstructionVisitor& visitor) override;
    };

    struct Cmp : public Instruction {
        VirtualRegister a;
        VirtualRegister b;
        bool is_float;

        ~Cmp() = default;

        std::string to_str() const override;

        std::vector<VirtualRegister> defs() const override;

        std::vector<VirtualRegister> uses() const override;

        void accept(InstructionVisitor& visitor) override;
    };

    struct Lea : public Instruction {
        VirtualRegister source;
        VirtualRegister dest;
        bool is_float;

        Lea(VirtualRegister source, VirtualRegister dest);

        ~Lea() = default;

        std::string to_str() const override;

        std::vector<VirtualRegister> defs() const override;

        std::vector<VirtualRegister> uses() const override;

        void accept(InstructionVisitor& visitor) override;
    };

    struct LessEqual : public Instruction {
        VirtualRegister reg;
        bool is_float;

        ~LessEqual() = default;

        std::string to_str() const override;

        std::vector<VirtualRegister> defs() const override;

        std::vector<VirtualRegister> uses() const override;

        void accept(InstructionVisitor& visitor) override;
    };

    struct Greater : public Instruction {
        VirtualRegister reg;
        bool is_float;

        ~Greater() = default;

        std::string to_str() const override;

        std::vector<VirtualRegister> defs() const override;

        std::vector<VirtualRegister> uses() const override;

        void accept(InstructionVisitor& visitor) override;
    };

    struct GreaterEqual : public Instruction {
        VirtualRegister reg;
        bool is_float;

        ~GreaterEqual() = default;

        std::string to_str() const override;

        std::vector<VirtualRegister> defs() const override;

        std::vector<VirtualRegister> uses() const override;

        void accept(InstructionVisitor& visitor) override;
    };

    struct NotEquals : public Instruction {
        VirtualRegister reg;
        bool is_float;

        ~NotEquals() = default;

        std::string to_str() const override;

        std::vector<VirtualRegister> defs() const override;

        std::vector<VirtualRegister> uses() const override;

        void accept(InstructionVisitor& visitor) override;
    };
}
}
