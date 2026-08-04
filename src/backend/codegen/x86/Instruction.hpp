#pragma once

#include "codegen/x86/MachineBlock.hpp"
#include "codegen/x86/Operand.hpp"
#include "codegen/x86/Register.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace jl {
namespace x86 {

    struct Instruction {
        uint32_t m_id = 0;

        virtual ~Instruction() = default;

        virtual std::string to_string() const = 0;

        virtual std::vector<VirtualRegister> defs() const = 0;

        virtual std::vector<VirtualRegister> uses() const = 0;

        virtual void replace(VirtualRegister reg, Operand operand) = 0;
    };

    struct Mov : public Instruction {
        Operand source;
        Operand dest;
        bool is_float;

        ~Mov() = default;

        inline std::string to_string() const override
        {
            OperandPrinter printer;
            return "mov "
                + std::visit(printer, dest)
                + ", " + std::visit(printer, source);
        }

        std::vector<VirtualRegister> defs() const override
        {
            return std::visit(GetDefinedRegs {}, dest);
        }

        std::vector<VirtualRegister> uses() const override
        {
            auto src_uses = std::visit(GetUsedRegs {}, source);
            if (std::holds_alternative<MemoryOperand>(dest)) {
                const auto dest_uses = std::visit(GetUsedRegs {}, dest);
                src_uses.insert(src_uses.end(), dest_uses.begin(), dest_uses.end());
            }

            return src_uses;
        }

        inline void replace(VirtualRegister reg, Operand operand) override
        {
            if (auto vreg = std::get_if<VirtualRegister>(&source); vreg && vreg->id == reg.id) {
                source = operand;
            }
            if (auto vreg = std::get_if<VirtualRegister>(&dest); vreg && vreg->id == reg.id) {
                dest = operand;
            }
            // std::visit(OperandReplacer(reg, operand), source, dest);
        }
    };

    struct Add : public Instruction {
        Operand source;
        Operand dest;
        bool is_float;

        ~Add() = default;

        inline std::string to_string() const override
        {
            OperandPrinter printer;
            return "add "
                + std::visit(printer, dest)
                + ", " + std::visit(printer, source);
        }

        std::vector<VirtualRegister> defs() const override
        {
            return std::visit(GetDefinedRegs {}, dest);
        }

        std::vector<VirtualRegister> uses() const override
        {
            auto src_uses = std::visit(GetUsedRegs {}, source);
            if (std::holds_alternative<MemoryOperand>(dest)) {
                const auto dest_uses = std::visit(GetUsedRegs {}, dest);
                src_uses.insert(src_uses.end(), dest_uses.begin(), dest_uses.end());
            }

            return src_uses;
        }

        inline void replace(VirtualRegister reg, Operand operand) override
        {
            if (auto vreg = std::get_if<VirtualRegister>(&source); vreg && vreg->id == reg.id) {
                source = operand;
            }
            if (auto vreg = std::get_if<VirtualRegister>(&dest); vreg && vreg->id == reg.id) {
                dest = operand;
            }
        }
    };

    struct Sub : public Instruction {
        Operand source;
        Operand dest;
        bool is_float;

        ~Sub() = default;

        inline std::string to_string() const override
        {
            OperandPrinter printer;
            return "sub "
                + std::visit(printer, dest)
                + ", " + std::visit(printer, source);
        }

        std::vector<VirtualRegister> defs() const override
        {
            return std::visit(GetDefinedRegs {}, dest);
        }

        std::vector<VirtualRegister> uses() const override
        {
            auto src_uses = std::visit(GetUsedRegs {}, source);
            if (std::holds_alternative<MemoryOperand>(dest)) {
                const auto dest_uses = std::visit(GetUsedRegs {}, dest);
                src_uses.insert(src_uses.end(), dest_uses.begin(), dest_uses.end());
            }

            return src_uses;
        }

        inline void replace(VirtualRegister reg, Operand operand) override
        {
            if (auto vreg = std::get_if<VirtualRegister>(&source); vreg && vreg->id == reg.id) {
                source = operand;
            }
            if (auto vreg = std::get_if<VirtualRegister>(&dest); vreg && vreg->id == reg.id) {
                dest = operand;
            }
        }
    };

    struct Less : public Instruction {
        VirtualRegister reg;
        bool is_float;

        ~Less() = default;

        inline std::string to_string() const override
        {
            return "setl " + reg.to_string();
        }

        std::vector<VirtualRegister> defs() const override
        {
            return { reg };
        }

        std::vector<VirtualRegister> uses() const override
        {
            return {};
        }

        inline void replace(VirtualRegister r, Operand operand) override
        {
            if (reg.id == r.id) {
                // if (auto vreg = std::get_if<VirtualRegister>(&operand)) {
                //     reg = *vreg;
                // }
                reg = std::get<VirtualRegister>(operand);
            }
        }
    };

    struct Equals : public Instruction {
        VirtualRegister reg;
        bool is_float;

        ~Equals() = default;

        inline std::string to_string() const override
        {
            return "sete " + reg.to_string();
        }

        std::vector<VirtualRegister> defs() const override
        {
            return { reg };
        }

        std::vector<VirtualRegister> uses() const override
        {
            return {};
        }

        inline void replace(VirtualRegister r, Operand operand) override
        {
            if (reg.id == r.id) {
                reg = std::get<VirtualRegister>(operand);
            }
        }
    };

    struct Return : public Instruction {
        ~Return() = default;

        inline std::string to_string() const override
        {
            return "ret";
        }

        std::vector<VirtualRegister> defs() const override
        {
            return {};
        }

        std::vector<VirtualRegister> uses() const override
        {
            return {};
        }

        inline void replace(VirtualRegister, Operand) override { }
    };

    struct Push : public Instruction {
        Operand value;

        ~Push() = default;

        inline std::string to_string() const override
        {
            return "push " + std::visit(OperandPrinter {}, value);
        }

        std::vector<VirtualRegister> defs() const override
        {
            return {};
        }

        std::vector<VirtualRegister> uses() const override
        {
            return std::visit(GetUsedRegs {}, value);
        }

        inline void replace(VirtualRegister r, Operand operand) override
        {
            if (auto vreg = std::get_if<VirtualRegister>(&value); vreg) {
                if (vreg->id == r.id) {
                    value = operand;
                }
            }
        }
    };

    struct Pop : public Instruction {
        Operand value;

        ~Pop() = default;

        inline std::string to_string() const override
        {
            return "pop " + std::visit(OperandPrinter {}, value);
        }

        std::vector<VirtualRegister> defs() const override
        {
            return std::visit(GetDefinedRegs {}, value);
        }

        std::vector<VirtualRegister> uses() const override
        {
            if (std::holds_alternative<MemoryOperand>(value)) {
                return std::visit(GetUsedRegs {}, value);
            }
            return {};
        }

        inline void replace(VirtualRegister r, Operand operand) override
        {
            if (auto vreg = std::get_if<VirtualRegister>(&value); vreg) {
                if (vreg->id == r.id) {
                    value = operand;
                }
            }
        }
    };

    struct Jump : public Instruction {
        MachineBlock* target;

        ~Jump() = default;

        inline std::string to_string() const override
        {
            return "jmp " + target->m_name;
        }

        std::vector<VirtualRegister> defs() const override
        {
            return {};
        }

        std::vector<VirtualRegister> uses() const override
        {
            return {};
        }

        inline void replace(VirtualRegister reg, Operand operand) override { }
    };

    struct JumpEqual : public Jump {
        ~JumpEqual() = default;

        inline std::string to_string() const override
        {
            return "je " + target->m_name;
        }

        std::vector<VirtualRegister> defs() const override
        {
            return {};
        }

        std::vector<VirtualRegister> uses() const override
        {
            return {};
        }

        inline void replace(VirtualRegister reg, Operand operand) override { }
    };

    struct Cmp : public Instruction {
        Operand a;
        Operand b;
        bool is_float;

        ~Cmp() = default;

        inline std::string to_string() const override
        {
            OperandPrinter printer;
            return "cmp "
                + std::visit(printer, a)
                + ", " + std::visit(printer, b);
        }

        std::vector<VirtualRegister> defs() const override
        {
            return {};
        }

        std::vector<VirtualRegister> uses() const override
        {
            auto a_uses = std::visit(GetUsedRegs {}, a);
            auto b_uses = std::visit(GetUsedRegs {}, b);
            a_uses.insert(a_uses.end(), b_uses.begin(), b_uses.end());
            return a_uses;
        }

        inline void replace(VirtualRegister r, Operand operand) override
        {
            if (auto vreg = std::get_if<VirtualRegister>(&a); vreg) {
                if (vreg->id == r.id) {
                    a = operand;
                }
            }
            if (auto vreg = std::get_if<VirtualRegister>(&b); vreg) {
                if (vreg->id == r.id) {
                    b = operand;
                }
            }
        }
    };

    struct Lea : public Instruction {
        MemoryOperand source;
        VirtualRegister dest;
        bool is_float;

        Lea(
            MemoryOperand source,
            VirtualRegister dest)
            : source(source)
            , dest(dest)
        {
        }

        ~Lea() = default;

        inline std::string to_string() const override
        {
            OperandPrinter printer;
            return "lea "
                + dest.to_string()
                + ", " + source.to_string();
        }

        std::vector<VirtualRegister> defs() const override
        {
            return { dest };
        }

        std::vector<VirtualRegister> uses() const override
        {
            return std::visit(GetUsedRegs {}, Operand(source));
        }

        inline void replace(VirtualRegister r, Operand operand) override
        {
            if (dest.id == r.id) {
                dest = std::get<VirtualRegister>(operand);
            }
            if (source.base.id == r.id) {
                source.base = std::get<VirtualRegister>(operand);
            }
            if (source.index && source.index->id == r.id) {
                source.index = std::get<VirtualRegister>(operand);
            }
        }
    };
}
}
