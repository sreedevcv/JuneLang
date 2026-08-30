#include "Passes.hpp"

#include "RegisterAllocator.hpp"
#include "codegen/x86/Instruction.hpp"
#include "codegen/x86/MachineFunction.hpp"
#include "codegen/x86/Operand.hpp"
#include "codegen/x86/Register.hpp"

#include <cassert>
#include <cstdint>
#include <iterator>
#include <memory>
#include <variant>

jl::x86::MachineAlloc to_machine_alloc(jl::Allocation alloc,
    jl::x86::MachineFunction* function,
    const jl::x86::VirtualRegister& vreg,
    uint32_t size)
{
    switch (alloc.type) {
    case jl::Allocation::GPR:
    case jl::Allocation::FLOAT: {
        return jl::x86::PhysicalRegister(
            static_cast<jl::x86::PhysicalRegister::Type>(alloc.value),
            vreg.size == jl::x86::SizeDirective::BYTE);
    }
    case jl::Allocation::SLOT: {
        jl::x86::MemoryOperand stack_source;
        auto base_reg = function->get_physical_register(jl::x86::PhysicalRegister::rbp);
        stack_source.base = base_reg;
        stack_source.displacement = -(alloc.value + size);
        stack_source.index = std::nullopt;
        if (auto directive = jl::x86::is_simple_move(size)) {
            stack_source.size = directive;
        }
        return stack_source;
    } break;
    }
    return {};
}

// Move the function params from register to stack if they are allocated in stack
void move_inputs_to_stk_if_needed(jl::x86::MachineFunction* function, const jl::x86::pass::AllocationMap& allocations)
{
    std::vector<std::unique_ptr<jl::x86::Instruction>> moves;

    // Move the input arguments to the allocated regs/stacks
    int float_count = 0;
    int gpr_count = 0;

    for (int i = 0; i < function->inputs().size(); i++) {
        auto param = function->inputs()[i];
        const auto& alloc = allocations.at(param);

        if (alloc.type != jl::Allocation::SLOT)
            continue;

        auto source = function->new_register();
        auto dest = function->new_register();
        function->set_allocation(dest, *function->get_allocation(param));
        if (param.is_float) {
            function->set_allocation(source, jl::x86::PhysicalRegister(jl::x86::input_float_registers[float_count++]));
        } else {
            function->set_allocation(source, jl::x86::PhysicalRegister(jl::x86::input_gpr_registers[gpr_count++]));
        }

        auto move = new jl::x86::Mov();
        move->source = source;
        move->dest = dest;

        moves.emplace_back(move);
    }

    auto& entry = function->blocks().front()->m_instructions;
    for (auto& mov : moves) {
        entry.insert(entry.begin(), std::move(mov));
    }
}

bool is_memory_operand(const jl::x86::MachineAlloc& alloc)
{
    return std::get_if<jl::x86::MemoryOperand>(&alloc) != nullptr
        || std::get_if<jl::x86::MemoryLabel>(&alloc) != nullptr;
}

// Rewrite addsd/subsd instructions where either source is a memory operand or both source and
// dest are memory operand
void rewrite_sd_instr_with_mem_as_source(jl::x86::MachineFunction* function)
{
    const auto scratch = function->get_physical_register(jl::x86::PhysicalRegister::xmm15);

    for (auto& block : function->blocks()) {
        for (auto iter = block->m_instructions.begin(); iter != block->m_instructions.end(); ++iter) {
            auto binary = dynamic_cast<jl::x86::Binary*>(iter->get());

            if (!binary)
                continue;
            if (!binary->is_float)
                continue;

            if (dynamic_cast<jl::x86::Cmp*>(binary)) {
                // ucomisd instructions cannot have a memory operand as the first operand
                const auto dest = *function->get_allocation(binary->dest);
                if (!is_memory_operand(dest))
                    continue;

                // Move the first memory operand to the scratch register and then do the cmp
                auto new_move = std::make_unique<jl::x86::Mov>();
                new_move->source = binary->dest;
                new_move->dest = scratch;
                new_move->is_float = true;
                block->m_instructions.insert(iter, std::move(new_move));

                binary->dest = scratch;
                continue;
            }

            if (!dynamic_cast<jl::x86::Add*>(binary) && !dynamic_cast<jl::x86::Sub*>(binary))
                continue;

            const auto dest = *function->get_allocation(binary->dest);
            const auto source = *function->get_allocation(binary->source);

            if (is_memory_operand(dest)) {
                // The previous instruction should be a move instruction.
                // Change the destination of that move to scratch register
                auto prev = static_cast<jl::x86::Mov*>(std::prev(iter)->get());
                if (!prev)
                    continue;

                prev->dest = scratch;
                // Change the current instructions dest to be the scracth register
                const auto original_dest = binary->dest;
                binary->dest = scratch;
                // Add a new mov instruction to move the result from the scratch
                // register to the original memory operand
                auto new_move = std::make_unique<jl::x86::Mov>();
                new_move->source = scratch;
                new_move->dest = binary->dest;
                new_move->is_float = true;
                block->m_instructions.insert(std::next(iter), std::move(new_move));
            }
        }
    }
}

// Rewrite move instructions where both source and destination is a memory operand
void rewrite_mem_to_mem_moves(jl::x86::MachineFunction* function)
{
    auto gpr_scratch = function->get_physical_register(jl::x86::PhysicalRegister::rax);
    auto float_scratch = function->get_physical_register(jl::x86::PhysicalRegister::xmm15);

    for (auto& block : function->blocks()) {
        for (auto iter = block->m_instructions.begin(); iter != block->m_instructions.end(); ++iter) {
            auto binary = dynamic_cast<jl::x86::Binary*>(iter->get());

            if (!binary)
                continue;

            const auto dest = *function->get_allocation(binary->dest);
            const auto source = *function->get_allocation(binary->source);

            if (!is_memory_operand(dest) || !is_memory_operand(source))
                continue;

            // insert a mov from source to scratch register
            auto new_move = new jl::x86::Mov();
            new_move->dest = binary->is_float ? float_scratch : gpr_scratch;
            new_move->source = binary->source;
            new_move->is_float = binary->is_float;
            block->m_instructions.insert(iter, std::unique_ptr<jl::x86::Instruction> { new_move });
            // Change the source in the existing instruction to the scratch register
            binary->source = binary->is_float ? float_scratch : gpr_scratch;
        }
    }
}

void add_prologue_and_epilogue(jl::x86::MachineFunction* function)
{
    // Insert prologue
    auto entry = function->get_block(function->name());

    auto push_instr = new jl::x86::Push();
    push_instr->value = function->get_physical_register(jl::x86::PhysicalRegister::rbp);

    auto mov_instr = new jl::x86::Mov();
    mov_instr->dest = function->get_physical_register(jl::x86::PhysicalRegister::rbp);
    mov_instr->source = function->get_physical_register(jl::x86::PhysicalRegister::rsp);
    mov_instr->is_float = false;

    auto stack_size_reg = function->new_register();
    function->set_allocation(stack_size_reg, static_cast<int64_t>(function->total_stack_space));

    auto sub_instr = new jl::x86::Sub();
    sub_instr->dest = function->get_physical_register(jl::x86::PhysicalRegister::rsp);
    sub_instr->source = stack_size_reg;
    sub_instr->is_float = false;

    entry->m_instructions.push_back(std::unique_ptr<jl::x86::Instruction>(std::move(push_instr)));
    entry->m_instructions.push_back(std::unique_ptr<jl::x86::Instruction>(std::move(mov_instr)));
    entry->m_instructions.push_back(std::unique_ptr<jl::x86::Instruction>(std::move(sub_instr)));

    auto front = std::move(function->blocks().back());
    function->blocks().pop_back();
    function->blocks().push_front(std::move(front));

    // Insert epilogue block
    auto eplg_mov_instr = new jl::x86::Mov();
    eplg_mov_instr->dest = function->get_physical_register(jl::x86::PhysicalRegister::rsp);
    eplg_mov_instr->source = function->get_physical_register(jl::x86::PhysicalRegister::rbp);
    mov_instr->is_float = false;

    auto pop_instr = new jl::x86::Pop();
    pop_instr->value = function->get_physical_register(jl::x86::PhysicalRegister::rbp);

    auto ret_instr = new jl::x86::Return;

    function->blocks().back()->m_instructions.emplace_back(eplg_mov_instr);
    function->blocks().back()->m_instructions.emplace_back(pop_instr);
    function->blocks().back()->m_instructions.emplace_back(ret_instr);
}

void remove_redundant_instrs(jl::x86::MachineFunction* function)
{
    for (auto& block : function->blocks()) {
        for (auto iter = block->m_instructions.begin(); iter != block->m_instructions.end();) {
            auto binary = dynamic_cast<jl::x86::Binary*>(iter->get());

            if (binary) {
                const auto dest = *function->get_allocation(binary->dest);
                const auto source = *function->get_allocation(binary->source);

                if (dest == source) {
                    auto next = std::next(iter);
                    block->m_instructions.erase(iter);
                    iter = next;
                    continue;
                }
            }
            ++iter;
        }
    }
}

void jl::x86::pass::assign_register(jl::x86::MachineFunction* function, AllocationMap allocations)
{
    using namespace jl;

    for (auto [vreg, alloc] : allocations) {
        auto var = *function->get_variable(vreg);
        auto machine_alloc = to_machine_alloc(alloc, function, vreg, var.type()->size());
        function->set_allocation(vreg, machine_alloc);
    }

    move_inputs_to_stk_if_needed(function, allocations);

    // This ordering is important!
    rewrite_sd_instr_with_mem_as_source(function);
    rewrite_mem_to_mem_moves(function);

    remove_redundant_instrs(function);
    add_prologue_and_epilogue(function);
}
