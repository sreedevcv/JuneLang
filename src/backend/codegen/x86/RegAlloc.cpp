#include "Passes.hpp"

#include "codegen/x86/Instruction.hpp"
#include "codegen/x86/MachineAlloc.hpp"
#include "codegen/x86/MachineFunction.hpp"
#include "codegen/x86/Operand.hpp"
#include "codegen/x86/Register.hpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <memory>
#include <optional>
#include <variant>

jl::x86::MachineAlloc to_machine_alloc(jl::x86::Allocation alloc,
    jl::x86::MachineFunction* function,
    const jl::x86::VirtualRegister& vreg)
{
    switch (alloc.type) {
    case jl::x86::Allocation::GPR:
    case jl::x86::Allocation::FLOAT: {
        return jl::x86::PhysicalRegister(
            static_cast<jl::x86::PhysicalRegister::Type>(alloc.value),
            vreg.size == jl::x86::SizeDirective::BYTE);
    }
    case jl::x86::Allocation::SLOT: {
        jl::x86::MemoryOperand stack_source;
        auto base_reg = function->get_physical_register(jl::x86::PhysicalRegister::rbp);
        stack_source.base = base_reg;
        stack_source.displacement = -(alloc.value + *size_directive_to_int(vreg.size));
        stack_source.index = std::nullopt;
        stack_source.size = vreg.size;
        return stack_source;
    } break;
    }
    return {};
}

// Move the function params from register to stack if they are allocated in stack
void move_inputs_to_stk_if_needed(jl::x86::MachineFunction* function, const jl::x86::AllocationMap& allocations)
{
    std::vector<std::unique_ptr<jl::x86::Instruction>> moves;

    // Move the input arguments to the allocated regs/stacks
    int float_count = 0;
    int gpr_count = 0;

    for (int i = 0; i < function->inputs().size(); i++) {
        auto param = function->inputs()[i];
        const auto& alloc = allocations.at(param);

        if (alloc.type != jl::x86::Allocation::SLOT)
            continue;

        auto source = function->new_register(param.size, param.is_float);
        auto dest = function->new_register(param.size, param.is_float);
        function->set_allocation(dest, *function->get_allocation(param));
        if (param.is_float) {
            function->set_allocation(source, jl::x86::PhysicalRegister(jl::x86::input_float_registers[float_count++]));
        } else {
            function->set_allocation(source, jl::x86::PhysicalRegister(jl::x86::input_gpr_registers[gpr_count++]));
        }

        auto move = new jl::x86::Mov();
        move->source = source;
        move->dest = dest;
        move->is_float = param.is_float;

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

            if (!(binary && binary->is_float))
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

            if (!dynamic_cast<jl::x86::Add*>(binary)
                && !dynamic_cast<jl::x86::Sub*>(binary)
                && !dynamic_cast<jl::x86::Mul*>(binary)
                && !dynamic_cast<jl::x86::Div*>(binary)) {
                continue;
            }

            const auto dest = *function->get_allocation(binary->dest);

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
                new_move->dest = original_dest;
                new_move->is_float = true;
                block->m_instructions.insert(std::next(iter), std::move(new_move));
            }
        }
    }
}

// Rewrite move instructions where both source and destination is a memory operand
void rewrite_mem_to_mem_moves(jl::x86::MachineFunction* function)
{
    auto gpr_scratch = jl::x86::PhysicalRegister::rax;
    auto float_scratch = jl::x86::PhysicalRegister::xmm15;

    for (auto& block : function->blocks()) {
        for (auto iter = block->m_instructions.begin(); iter != block->m_instructions.end(); ++iter) {
            auto binary = dynamic_cast<jl::x86::Binary*>(iter->get());

            if (!binary)
                continue;

            const auto dest = *function->get_allocation(binary->dest);
            const auto source = *function->get_allocation(binary->source);

            if (!is_memory_operand(dest) || !is_memory_operand(source))
                continue;

            assert(binary->source.size == binary->dest.size);

            auto scratch = function->new_register(binary->source.size);
            function->set_allocation(scratch, jl::x86::PhysicalRegister(binary->is_float ? float_scratch : gpr_scratch, binary->source.size == jl::x86::SizeDirective::BYTE));

            // insert a mov from source to scratch register
            auto new_move = new jl::x86::Mov();
            new_move->dest = scratch;
            new_move->source = binary->source;
            new_move->is_float = binary->is_float;
            block->m_instructions.insert(iter, std::unique_ptr<jl::x86::Instruction> { new_move });
            // Change the source in the existing instruction to the scratch register
            binary->source = scratch;
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

    auto stack_size_reg = function->new_register(jl::x86::SizeDirective::NONE);
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

std::vector<std::unique_ptr<jl::x86::Mov>> parallel_moves(jl::x86::MachineFunction* function,
    std::vector<jl::x86::VirtualRegister> srcs,
    std::vector<jl::x86::VirtualRegister> dests,
    jl::x86::VirtualRegister scratch,
    bool is_float)
{
    enum class Status {
        TO_MOVE,
        BEING_MOVED,
        MOVED
    };

    const uint32_t n = srcs.size();
    std::vector<Status> status(n, Status::TO_MOVE);
    std::vector<std::unique_ptr<jl::x86::Mov>> moves;

    const auto make_move = [&](const jl::x86::VirtualRegister& src_reg, const jl::x86::VirtualRegister& dest_reg) {
        auto mov = std::make_unique<jl::x86::Mov>();
        mov->source = src_reg;
        mov->dest = dest_reg;
        mov->is_float = is_float;

        moves.push_back(std::move(mov));
    };

    const auto eq = [&](const jl::x86::VirtualRegister& src_reg, const jl::x86::VirtualRegister& dest_reg) {
        const auto a = *function->get_allocation(src_reg);
        const auto b = *function->get_allocation(dest_reg);
        return a == b;
    };

    const auto move_one = [&](this auto& self, uint32_t i) {
        // No need to generate a move if both the srcs and destsination are already the same
        if (eq(srcs[i], dests[i])) {
            return;
        }

        status[i] = Status::BEING_MOVED;

        for (uint32_t j = 0; j < n; j++) {
            // If some other move's srcs is the destsination that we are currently considering,
            if (eq(dests[i], srcs[j])) {
                switch (status[j]) {
                case Status::TO_MOVE:
                    // then make that move first
                    self(j);
                    break;
                case Status::BEING_MOVED:
                    // This is a cycle, resolve it using a scratch register
                    make_move(srcs[j], scratch);
                    // Keep track the new value of src
                    srcs[j] = scratch;
                    break;
                case Status::MOVED:
                    // alread moved - nothing to do
                    break;
                }
            }
        }

        make_move(srcs[i], dests[i]);
        status[i] = Status::MOVED;
    };

    for (uint32_t i = 0; i < n; i++) {
        if (status[i] == Status::TO_MOVE) {
            move_one(i);
        }
    }

    return moves;
}

void collect_input_regs(std::vector<jl::x86::VirtualRegister>& srcs,
    std::vector<jl::x86::VirtualRegister>& dests,
    const std::vector<jl::x86::VirtualRegister>& args,
    jl::x86::MachineFunction* function,
    bool is_float)
{
    uint32_t count = 0;
    for (const auto& reg : args) {
        if (!is_float && !reg.is_float) {
            auto phy_reg = jl::x86::PhysicalRegister(jl::x86::input_gpr_registers[count++]);
            auto dest = function->new_register(reg.size);
            function->set_allocation(dest, phy_reg);

            dests.push_back(dest);
            srcs.push_back(reg);
        }
        if (is_float && reg.is_float) {
            auto phy_reg = jl::x86::PhysicalRegister(jl::x86::input_float_registers[count++]);
            auto dest = function->new_register(reg.size);
            function->set_allocation(dest, phy_reg);

            dests.push_back(dest);
            srcs.push_back(reg);
        }
    }
}

void move_function_args_to_input_regs(jl::x86::MachineFunction* function, const jl::x86::AllocationResult& allocation_result)
{
    for (auto& block : function->blocks()) {
        for (auto iter = block->m_instructions.begin(); iter != block->m_instructions.end(); ++iter) {
            auto call = dynamic_cast<jl::x86::Call*>(iter->get());

            if (call == nullptr) {
                continue;
            }

            const auto& active = allocation_result.active_at_call_sites.at(call->ret_value);

            std::vector<jl::x86::VirtualRegister> active_regs;
            std::ranges::transform(active, std::back_inserter(active_regs), [&function](const auto preg) {
                const auto reg = jl::x86::PhysicalRegister(preg);
                auto vreg = function->new_register(jl::x86::SizeDirective::QWORD,
                    reg.is_float());
                function->set_allocation(vreg, reg);
                return vreg;
            });

            uint32_t float_offset_count = 0;
            auto float_arg_count = std::ranges::count_if(call->args, [](auto&& reg) { return reg.is_float; });
            uint32_t bytes = float_arg_count * 8;
            uint32_t reserved = (bytes + 15) & ~15; // round up to multiple of 16
            std::optional<jl::x86::VirtualRegister> count_reg = std::nullopt;

            // xmm registers cant be pushed to the stack during push, so we will manually move them onto the stack
            if (float_arg_count > 0) {
                auto creg = function->new_register(jl::x86::SizeDirective::QWORD);
                function->set_allocation(creg, reserved);

                auto sub = std::make_unique<jl::x86::Sub>();
                sub->dest = function->get_physical_register(jl::x86::PhysicalRegister::rsp);
                sub->source = creg;
                sub->is_float = false;
                block->m_instructions.insert(iter, std::move(sub));

                count_reg = creg;
            }

            // Push/Move the active registers
            for (const auto reg : active_regs) {
                if (reg.is_float) {
                    auto stack = jl::x86::MemoryOperand();
                    stack.base = function->get_physical_register(jl::x86::PhysicalRegister::rbp);
                    stack.displacement = float_offset_count;
                    stack.index = std::nullopt;
                    float_offset_count += 8;

                    auto dest = function->new_register(reg.size);
                    function->set_allocation(dest, stack);
                    auto mov = std::make_unique<jl::x86::Mov>();
                    mov->dest = dest;
                    mov->source = reg;
                    mov->is_float = true;

                    block->m_instructions.insert(iter, std::move(mov));
                } else {
                    auto push = std::make_unique<jl::x86::Push>();
                    push->value = reg;
                    block->m_instructions.insert(iter, std::move(push));
                }
            }

            float_offset_count = 0;

            // Pop the active registers
            for (const auto reg : active_regs) {
                // We will be popping only after the intr that moves the return value from the return register
                auto insert_iter = std::next(std::next(iter));

                if (reg.is_float) {
                    auto stack = jl::x86::MemoryOperand();
                    stack.base = function->get_physical_register(jl::x86::PhysicalRegister::rbp);
                    stack.displacement = float_offset_count;
                    stack.index = std::nullopt;
                    float_offset_count += 8;

                    auto src = function->new_register(reg.size);
                    function->set_allocation(src, stack);
                    auto mov = std::make_unique<jl::x86::Mov>();
                    mov->dest = reg;
                    mov->source = src;
                    mov->is_float = true;

                    block->m_instructions.insert(insert_iter, std::move(mov));
                } else {
                    auto pop = std::make_unique<jl::x86::Pop>();
                    pop->value = reg;
                    block->m_instructions.insert(insert_iter, std::move(pop));
                }
            }

            if (float_arg_count > 0) {
                auto add = std::make_unique<jl::x86::Mov>();
                add->dest = function->get_physical_register(jl::x86::PhysicalRegister::rsp);
                add->source = *count_reg;
                add->is_float = false;

                auto add_iter = std::next(iter, active_regs.size() + 1);
                block->m_instructions.insert(add_iter, std::move(add));
            }

            std::println("Call: {}", call->function_name);
            std::vector<jl::x86::VirtualRegister> srcs;
            std::vector<jl::x86::VirtualRegister> dests;
            collect_input_regs(srcs, dests, call->args, function, false);

            for (int i = 0; i < srcs.size(); i++) {
                auto s = std::visit(jl::x86::MachineAllocPrinter(function), *function->get_allocation(srcs[i]));
                auto d = std::visit(jl::x86::MachineAllocPrinter(function), *function->get_allocation(dests[i]));
                println("mov {} <- {}", d, s);
            }
            std::println("Generated moves: ");

            auto rax = function->get_physical_register(jl::x86::PhysicalRegister::rax);
            auto moves1 = parallel_moves(function, srcs, dests, rax, false);
            for (auto& move : moves1) {
                // std::println("gen = {}", mov.)
                block->m_instructions.insert(iter, std::move(move));
            }

            srcs.clear();
            dests.clear();

            collect_input_regs(srcs, dests, call->args, function, true);
            auto xmm15 = function->get_physical_register(jl::x86::PhysicalRegister::xmm15);
            auto moves2 = parallel_moves(function, srcs, dests, xmm15, true);
            for (auto& move : moves2)
                block->m_instructions.insert(iter, std::move(move));
        }
    }
}

void jl::x86::pass::assign_register(jl::x86::MachineFunction* function, const AllocationResult& allocation_result)
{
    using namespace jl;

    for (auto [vreg, alloc] : allocation_result.allocations) {
        auto machine_alloc = to_machine_alloc(alloc, function, vreg);
        //      std::println("vreg: {}, var: {}, alloc: {}, maachalloc: {}", vreg.to_str(), var.to_str(), alloc.to_str(),
        //          std::visit(jl::x86::MachineAllocPrinter(function), machine_alloc));
        function->set_allocation(vreg, machine_alloc);
    }

    move_function_args_to_input_regs(function, allocation_result);
    move_inputs_to_stk_if_needed(function, allocation_result.allocations);

    // This ordering is important!
    rewrite_sd_instr_with_mem_as_source(function);
    rewrite_mem_to_mem_moves(function);

    remove_redundant_instrs(function);
    std::cout << "---------------------------------------------------\n";
    std::cout << function->to_str();

    add_prologue_and_epilogue(function);
}
