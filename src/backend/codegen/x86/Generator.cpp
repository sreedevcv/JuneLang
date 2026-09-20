#include "Generator.hpp"

#include "Function.hpp"
#include "LiteralValue.hpp"
#include "Utils.hpp"
#include "codegen/x86/Instruction.hpp"
#include "codegen/x86/MachineAlloc.hpp"
#include "codegen/x86/MachineBlock.hpp"
#include "codegen/x86/MachineFunction.hpp"
#include "codegen/x86/Operand.hpp"
#include "codegen/x86/Register.hpp"
#include "ir/Binary.hpp"
#include "ir/Call.hpp"
#include "ir/ConditionalJump.hpp"
#include "ir/IR.hpp"
#include "ir/InitLiteral.hpp"
#include "ir/Jump.hpp"
#include "ir/Read.hpp"
#include "ir/Return.hpp"
#include "ir/Write.hpp"
#include "types/Type.hpp"

#include <cassert>
#include <cstdint>
#include <memory>
#include <optional>
#include <utility>
#include <variant>

jl::x86::Generator::Generator(jl::Function* function)
    : m_function(function)
    , m_out(function->name(), function)
{
    int gpr_count = 0;
    int float_count = 0;

    for (auto arg : function->args()) {
        assert(arg.type()->m_kind == type::Type::BUILTIN);

        const auto reg = m_out.get_register(arg);
        const auto preg = type::is_float(arg.type())
            ? input_float_registers[float_count++]
            : input_gpr_registers[gpr_count++];
        m_out.set_allocation(reg, PhysicalRegister(preg));
        m_out.inputs().push_back(reg);

        assert(float_count <= 6);
        assert(gpr_count <= 6);
    }
}

jl::x86::MachineFunction jl::x86::Generator::generate()
{
    // Create an epilogue block that can be reference by the return generator
    auto epilogue = std::make_unique<MachineBlock>(m_function->name() + "_epilogue");
    m_epilogue_block = epilogue.get();

    // Generate x86 instructions for each of the june ir
    for (auto& block : m_function->blocks()) {
        generate(block.get());
    }

    m_out.blocks().push_back(std::move(epilogue));

    return std::move(m_out);
}

void jl::x86::Generator::generate(BasicBlock* block)
{
    auto mblock = m_out.get_block(m_function->name() + "." + block->get_name());

    set_current_block(mblock);
    for (auto ir = block->head; ir != nullptr; ir = ir->next) {
        ir->accept(*this);
    }
}

void jl::x86::Generator::visit_binary_ir(ir::Binary& binary)
{
    auto a = m_out.get_register(binary.m_operand_a);
    auto b = m_out.get_register(binary.m_operand_b);
    auto& result = m_out.get_register(binary.m_dest);

    const auto generate_move_and_operation = [&](auto oper) {
        auto mov = new Mov();
        mov->dest = result;
        mov->source = a;
        mov->is_float = binary.m_is_float;

        oper->is_float = binary.m_is_float;
        oper->dest = result;
        oper->source = b;

        m_curr_block->m_instructions.emplace_back(mov);
        m_curr_block->m_instructions.emplace_back(oper);
    };

    const auto generate_cmp_and_move = [&](auto oper) {
        auto cmp = new Cmp();
        cmp->dest = a;
        cmp->source = b;
        cmp->is_float = binary.m_is_float;

        result.size = SizeDirective::BYTE;

        oper->reg = result;
        oper->is_float = binary.m_is_float;

        m_curr_block->m_instructions.emplace_back(cmp);
        m_curr_block->m_instructions.emplace_back(oper);
    };

    const auto generate_division = [&]() {
        if (binary.m_is_float) {
            generate_move_and_operation(new Div());
        } else {
            auto rax = m_out.new_register(SizeDirective::QWORD);
            m_out.set_allocation(rax, PhysicalRegister(PhysicalRegister::rax));

            auto mov = new Mov();
            mov->dest = rax;
            mov->source = a;
            mov->is_float = false;

            auto cqo = new Cqo();

            auto divisor = b;
            if (auto machine_alloc = m_out.get_allocation(b)) {
                auto alloc = *machine_alloc;
                if (std::get_if<int64_t>(&alloc) != nullptr) {
                    // we cant have the divisor be an immediate value, so we generate a mov
                    // to store it in a register
                    divisor = m_out.new_register(SizeDirective::QWORD);

                    auto mov = new Mov();
                    mov->source = b;
                    mov->dest = divisor;
                    mov->is_float = false;

                    m_curr_block->m_instructions.emplace_back(mov);
                }
            }

            auto div = new Div();
            div->source = divisor;
            div->dest = rax; // Just to indicate to the liveness analyser that div uses and defines the rax
            div->is_float = false;

            auto mov2 = new Mov();
            mov2->source = rax;
            mov2->dest = result;
            mov->is_float = false;
            m_curr_block->m_instructions.emplace_back(mov);
            m_curr_block->m_instructions.emplace_back(cqo);
            m_curr_block->m_instructions.emplace_back(div);
            m_curr_block->m_instructions.emplace_back(mov2);
        }
    };

    switch (binary.m_operation) {
    case ir::Binary::PLUS:
        generate_move_and_operation(new Add());
        return;
    case ir::Binary::MINUS:
        generate_move_and_operation(new Sub());
        return;
    case ir::Binary::STAR:
        generate_move_and_operation(new Mul());
        return;
    case ir::Binary::SLASH:
        generate_division();
        return;
    case ir::Binary::GREATER:
        generate_cmp_and_move(new Greater());
        return;
    case ir::Binary::LESS:
        generate_cmp_and_move(new Less());
        return;
    case ir::Binary::GREATER_EQUAL:
        generate_cmp_and_move(new GreaterEqual());
        return;
    case ir::Binary::LESS_EQUAL:
        generate_cmp_and_move(new LessEqual());
        return;
    case ir::Binary::EQUAL_EQUAL:
        generate_cmp_and_move(new Equals());
        return;
    case ir::Binary::BANG_EQUAL:
        generate_cmp_and_move(new NotEquals());
        return;
    case ir::Binary::BIT_AND:
    case ir::Binary::LOG_AND:
        generate_move_and_operation(new And());
        return;
    case ir::Binary::BIT_OR:
    case ir::Binary::LOG_OR:
        generate_move_and_operation(new Or());
        return;
    case ir::Binary::BIT_XOR:
        generate_move_and_operation(new Xor());
        return;
    case ir::Binary::PERCENT:
        break;
    }
    unimplemented();
}

void jl::x86::Generator::visit_move_ir(ir::Move&)
{
    unimplemented();
}

void jl::x86::Generator::visit_return_ir(ir::Return& ret)
{
    if (ret.m_ret_val) {
        auto reg = type::is_float(ret.m_ret_val->type())
            ? PhysicalRegister::xmm0
            : PhysicalRegister::rax;
        auto dest_reg = m_out.new_register(*is_simple_move(ret.m_ret_val->type()->size()));
        m_out.set_allocation(dest_reg, jl::x86::PhysicalRegister(reg, ret.m_ret_val->type()->size() == 1));

        auto mov = new Mov();
        mov->dest = dest_reg;
        mov->source = m_out.get_register(*ret.m_ret_val);
        mov->is_float = false;
        m_curr_block->m_instructions.emplace_back(mov);
    }

    auto jmp = new Jump();
    jmp->target = m_epilogue_block;

    m_curr_block->m_instructions.emplace_back(jmp);
}

void jl::x86::Generator::visit_call_ir(ir::Call& call)
{
    std::vector<VirtualRegister> args;
    for (const auto var : call.m_args) {
        args.push_back(m_out.get_register(var));
    }

    const auto is_float = type::is_float(call.m_dest.type());
    const auto ret = m_out.new_register(*is_simple_move(call.m_dest.type()->size()), is_float);
    const auto ret_reg = is_float ? PhysicalRegister::xmm0 : PhysicalRegister::rax;
    m_out.set_allocation(ret, PhysicalRegister(ret_reg));

    auto call_instr = new Call();
    call_instr->args = std::move(args);
    call_instr->ret_value = ret;
    call_instr->function_name = call.m_name;

    auto mov = new Mov();
    mov->source = ret;
    mov->dest = m_out.get_register(call.m_dest);
    mov->is_float = type::is_float(call.m_dest.type());

    m_curr_block->m_instructions.emplace_back(call_instr);
    m_curr_block->m_instructions.emplace_back(mov);
}

void jl::x86::Generator::visit_jump_ir(ir::Jump& jump)
{
    auto jmp = new Jump();
    jmp->target = m_out.get_block(m_function->name() + "." + jump.m_target->get_name());

    m_curr_block->m_instructions.emplace_back(jmp);
}

void jl::x86::Generator::visit_cond_jump_ir(ir::CondJump& jump)
{
    auto cmp = new Cmp();
    auto one = m_out.new_register(SizeDirective::BYTE);
    m_out.set_allocation(one, static_cast<int64_t>(1));

    cmp->dest = m_out.get_register(jump.m_condition);
    cmp->source = one;

    auto je = new JumpEqual();
    je->target = m_out.get_block(m_function->name() + "." + jump.m_true_target->get_name());

    auto jmp = new Jump();
    jmp->target = m_out.get_block(m_function->name() + "." + jump.m_false_target->get_name());

    m_curr_block->m_instructions.emplace_back(cmp);
    m_curr_block->m_instructions.emplace_back(je);
    m_curr_block->m_instructions.emplace_back(jmp);
}

void jl::x86::Generator::visit_unary_ir(ir::Unary&)
{
    unimplemented();
}

void jl::x86::Generator::visit_allocate_list_ir(ir::AllocateList&)
{
    unimplemented();
}

void jl::x86::Generator::visit_allocate_var_ir(ir::AllocateVar& allocate)
{
    MemoryOperand stack_source;
    stack_source.base = m_out.get_physical_register(PhysicalRegister::rbp);
    stack_source.displacement = get_stack_offset(allocate.m_addr);
    stack_source.index = std::nullopt;

    if (auto directive = is_simple_move(allocate.m_var_type->size())) {
        stack_source.size = directive;
    } else {
        unimplemented("memcpy");
    }

    m_out.set_allocation(m_out.get_register(allocate.m_addr), stack_source);
}

void jl::x86::Generator::visit_read_ir(ir::Read& read)
{
    assert(!read.m_offset.has_value());

    auto mem_operand = std::get<MemoryOperand>(*m_out.get_allocation(m_out.get_register(read.m_base)));
    auto new_mem_operand = mem_operand;

    auto source = m_out.new_register(*is_simple_move(read.m_size));
    m_out.set_allocation(source, new_mem_operand);

    auto mov = new Mov();
    mov->source = source;
    mov->dest = m_out.get_register(read.m_dest);
    mov->is_float = false;

    m_curr_block->m_instructions.emplace_back(mov);
}

void jl::x86::Generator::visit_write_ir(ir::Write& write)
{
    assert(!write.m_offset.has_value());

    auto dest = m_out.new_register(*is_simple_move(write.m_size));
    auto mem_operand = std::get<MemoryOperand>(*m_out.get_allocation(m_out.get_register(write.m_base)));
    auto new_mem_operand = mem_operand;

    m_out.set_allocation(dest, new_mem_operand);

    auto mov = new Mov();
    mov->dest = dest;
    mov->source = m_out.get_register(write.m_src);
    mov->is_float = false;

    m_curr_block->m_instructions.emplace_back(mov);
}

void jl::x86::Generator::visit_init_literal_ir(ir::InitLiteral& literal)
{
    auto literal_reg = m_out.get_register(literal.m_dest);

    struct LiteralValueVisitor {
        MachineFunction& function;

        LiteralValueVisitor(MachineFunction& function)
            : function(function)
        {
        }

        MachineAlloc operator()(int64_t val) const
        {
            return val;
        }

        MachineAlloc operator()(double val) const
        {
            return function.add_float_to_data_section(val);
        }

        MachineAlloc operator()(bool val) const
        {
            return static_cast<int64_t>(val ? 1 : 0);
        }

        MachineAlloc operator()(char val) const
        {
            return static_cast<int64_t>(val);
        }
    };

    const auto alloc = std::visit(LiteralValueVisitor(m_out), literal.m_source.data);
    m_out.set_allocation(literal_reg, alloc);
}

void jl::x86::Generator::visit_debug_print_ir(ir::DebugPrint&)
{
    unimplemented();
}

void jl::x86::Generator::visit_type_cast_ir(ir::TypeCast&)
{
    unimplemented();
}

void jl::x86::Generator::visit_phi(ir::Phi&)
{
    unimplemented();
}

void jl::x86::Generator::set_current_block(MachineBlock* block)
{
    m_curr_block = block;
}

uint32_t jl::x86::Generator::get_stack_offset(value::Variable var)
{
    return -m_out.get_ssa_offset(var) - var.type()->size();
}
