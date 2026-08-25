#include "Generator.hpp"

#include "Function.hpp"
#include "LiteralValue.hpp"
#include "Utils.hpp"
#include "codegen/x86/Instruction.hpp"
#include "codegen/x86/MachineBlock.hpp"
#include "codegen/x86/Register.hpp"
#include "ir/Binary.hpp"
#include "ir/ConditionalJump.hpp"
#include "ir/IR.hpp"
#include "ir/InitLiteral.hpp"
#include "ir/Jump.hpp"
#include "ir/Read.hpp"
#include "ir/Return.hpp"
#include "ir/Write.hpp"
#include "types/Type.hpp"
#include <cassert>
#include <memory>
#include <optional>
#include <utility>

jl::x86::Generator::Generator(jl::Function* function)
    : m_function(function)
    , m_out(function->name(), function)
{
    assert(function->args().size() < 6 && "Need to move extra regs to stack");

    int count = 0;
    const PhysicalRegister::Type arg_registers[] = {
        PhysicalRegister::rdi,
        // PhysicalRegister::rsi,
        // PhysicalRegister::rdx,
        // PhysicalRegister::rcx,
        // PhysicalRegister::r8,
        // PhysicalRegister::r9
    };

    //  for (auto arg : function->args()) {
    //      assert(arg.type()->m_kind == type::Type::BUILTIN
    //          && static_cast<const type::Builtin*>(arg.type())->m_primitive != type::Builtin::FLOAT
    //          && "floats support");
    //      ;
    //      auto reg = m_out.map_register(arg, m_out.new_register(PhysicalRegister(arg_registers[count])));
    //      m_out.inputs().push_back(reg);
    //      count += 1;
    //  }
}

jl::x86::MachineFunction jl::x86::Generator::generate()
{
    // Insert prologue
    // auto entry = m_out.get_block(m_function->name() + "." + m_function->entry_block()->get_name());
    // m_out.map_physical_register(PhysicalRegister::rbp);
    // m_out.map_physical_register(PhysicalRegister::rsp);
    //
    // Push* push_instr = new Push();
    // push_instr->value = m_out.get_physical_register(PhysicalRegister::rbp);
    //
    // Mov* mov_instr = new Mov();
    // mov_instr->dest = m_out.get_physical_register(PhysicalRegister::rbp);
    // mov_instr->source = m_out.get_physical_register(PhysicalRegister::rsp);
    // mov_instr->is_float = false;
    //
    // Sub* sub_instr = new Sub();
    // sub_instr->dest = m_out.get_physical_register(PhysicalRegister::rsp);
    // sub_instr->source = m_out.total_stack_space;
    // sub_instr->is_float = false;
    //
    // entry->m_instructions.insert(entry->m_instructions.begin(), std::unique_ptr<Instruction>(std::move(sub_instr)));
    // entry->m_instructions.insert(entry->m_instructions.begin(), std::unique_ptr<Instruction>(std::move(mov_instr)));
    // entry->m_instructions.insert(entry->m_instructions.begin(), std::unique_ptr<Instruction>(std::move(push_instr)));

    // Create an epilogue block that can be reference by the return generator
    auto epilogue = std::make_unique<MachineBlock>(m_function->name() + "_epilogue");
    m_epilogue_block = epilogue.get();

    // Generate x86 instructions for each of the june ir
    for (auto& block : m_function->blocks()) {
        generate(block.get());
    }

    // Insert epilogue block
    // auto eplg_mov_instr = new Mov();
    // eplg_mov_instr->dest = m_out.new_register(PhysicalRegister(PhysicalRegister::rsp));
    // eplg_mov_instr->source = m_out.get_physical_register(PhysicalRegister::rbp);
    // mov_instr->is_float = false;
    //
    // auto pop_instr = new Pop();
    // pop_instr->value = m_out.get_physical_register(PhysicalRegister::rbp);
    //
    // auto ret_instr = new Return;
    //
    // epilogue->m_instructions.emplace_back(eplg_mov_instr);
    // epilogue->m_instructions.emplace_back(pop_instr);
    // epilogue->m_instructions.emplace_back(ret_instr);

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
    auto result = m_out.get_register(binary.m_dest);

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

        result.size = SizeDirective::QWORD;
        m_out.map_register(binary.m_dest, result);
    };

    const auto generate_cmp_and_move = [&](auto oper) {
        auto cmp = new Cmp();
        cmp->a = a;
        cmp->b = b;
        cmp->is_float = binary.m_is_float;

        oper->reg = result;
        oper->is_float = false;

        m_curr_block->m_instructions.emplace_back(cmp);
        m_curr_block->m_instructions.emplace_back(oper);

        result.size = SizeDirective::BYTE;
        m_out.map_register(binary.m_dest, result);
    };

    switch (binary.m_operation) {
    case ir::Binary::PLUS:
        generate_move_and_operation(new Add());
        return;
    case ir::Binary::MINUS:
        generate_move_and_operation(new Sub());
        return;
    case ir::Binary::STAR:
    case ir::Binary::SLASH:
    case ir::Binary::GREATER:
    case ir::Binary::LESS:
        generate_cmp_and_move(new Less());
        return;
    case ir::Binary::GREATER_EQUAL:
    case ir::Binary::LESS_EQUAL:
    case ir::Binary::EQUAL_EQUAL:
        generate_cmp_and_move(new Equals());
        return;
    case ir::Binary::BANG_EQUAL:
    case ir::Binary::PERCENT:
    case ir::Binary::BIT_AND:
    case ir::Binary::BIT_OR:
    case ir::Binary::BIT_XOR:
    case ir::Binary::LOG_AND:
    case ir::Binary::LOG_OR:
        break;
    }
    unimplemented();
}

void jl::x86::Generator::visit_move_ir(ir::Move& move)
{
    unimplemented();
}

void jl::x86::Generator::visit_return_ir(ir::Return& ret)
{
    if (ret.m_ret_val) {
        if (type::is_float(ret.m_ret_val->type())) {
            unimplemented("float return");
        }

        auto dest_reg = m_out.new_register();
        m_out.set_allocation(dest_reg, PhysicalRegister::rax);
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
    unimplemented();
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
    cmp->a = m_out.get_register(jump.m_condition);
    cmp->b = 1;

    auto je = new JumpEqual();
    je->target = m_out.get_block(m_function->name() + "." + jump.m_true_target->get_name());

    auto jmp = new Jump();
    jmp->target = m_out.get_block(m_function->name() + "." + jump.m_false_target->get_name());

    m_curr_block->m_instructions.emplace_back(cmp);
    m_curr_block->m_instructions.emplace_back(je);
    m_curr_block->m_instructions.emplace_back(jmp);
}

void jl::x86::Generator::visit_unary_ir(ir::Unary& unary)
{
    unimplemented();
}

void jl::x86::Generator::visit_label_ir(ir::Label& label)
{
    unimplemented();
}

void jl::x86::Generator::visit_allocate_list_ir(ir::AllocateList& allocate)
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

//  auto dest = m_out.new_register();
//  m_out.set_allocation(dest, stack_source);

//  auto lea = new Lea(dest, m_out.get_register(allocate.m_addr));
//  m_curr_block->m_instructions.emplace_back(lea);

    m_out.set_allocation(m_out.get_register(allocate.m_addr), stack_source);
}

void jl::x86::Generator::visit_read_ir(ir::Read& read)
{
    assert(!read.m_offset.has_value());

    auto mem_operand = std::get<MemoryOperand>(*m_out.get_allocation(m_out.get_register(read.m_base)));
    auto new_mem_operand = mem_operand;

    auto source = m_out.new_register();
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

    auto dest = m_out.new_register();
    auto mem_operand = std::get<MemoryOperand>(*m_out.get_allocation(m_out.get_register(write.m_base)));
    auto new_mem_operand = mem_operand;
    
    m_out.set_allocation(dest, new_mem_operand);

    auto mov = new Mov();
    mov->dest = dest;
    mov->source = m_out.get_register(write.m_src);
    mov->is_float = false;

//  if (auto directive = is_simple_move(write.m_size)) {
//      mov->size = directive;
//  } else {
//      unimplemented("memcpy");
//  }

    // MemoryOperand stack_source = std::get<MemoryOperand>(m_out.get_operand(write.m_base));
    // if (write.m_offset) {
    //     stack_source.index = m_out.get_register(*write.m_offset);
    //     stack_source.scale = write.m_offset_multiplier;
    // } else {
    //     stack_source.index = std::nullopt;
    // }
    // if (auto directive = is_simple_move(write.m_size)) {
    //     stack_source.size = directive;
    // } else {
    //     unimplemented("memcpy");
    // }

    // auto mov = new Mov();
    // mov->dest = stack_source;
    // mov->source = get_operand(write.m_src);
    // mov->is_float = false;

    // m_curr_block->m_instructions.emplace_back(mov);
}

void jl::x86::Generator::visit_init_literal_ir(ir::InitLiteral& literal)
{
    auto mov = new Mov();
    mov->dest = m_out.get_register(literal.m_dest);
    mov->source = std::get<LiteralValue::int_type>(literal.m_source.data);
    mov->is_float = false;
    mov->size = std::nullopt;

    m_curr_block->m_instructions.emplace_back(mov);
}

void jl::x86::Generator::visit_debug_print_ir(ir::DebugPrint& print)
{
    unimplemented();
}

void jl::x86::Generator::visit_type_cast_ir(ir::TypeCast& type_cast)
{
    unimplemented();
}

void jl::x86::Generator::visit_phi(ir::Phi& phi)
{
    unimplemented();
}

void jl::x86::Generator::set_current_block(MachineBlock* block)
{
    m_curr_block = block;
}

bool jl::x86::Generator::is_contant(ir::IR* ir) const
{
    return dynamic_cast<ir::InitLiteral*>(ir) ? true : false;
}

jl::ir::IR* jl::x86::Generator::get_def(value::Variable var)
{
    for (auto& ir : m_function->irs()) {
        if (ir->def() == var) {
            return ir.get();
        }
    }

    return nullptr;
}

jl::x86::Operand jl::x86::Generator::get_operand(value::Variable var)
{
    auto def = get_def(var);
    Operand a;
    if (is_contant(def)) {
        a = std::get<LiteralValue::int_type>(static_cast<ir::InitLiteral*>(def)->m_source.data);
    } else {
        a = m_out.get_operand(var);
    }

    return a;
}

uint32_t jl::x86::Generator::get_stack_offset(value::Variable var)
{
    return -m_out.get_ssa_offset(var) - var.type()->size();
}
