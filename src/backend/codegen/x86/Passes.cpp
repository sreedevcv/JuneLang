#include "codegen/x86/Passes.hpp"

#include "codegen/x86/Instruction.hpp"
#include "codegen/x86/LinearScanRegAllocator.hpp"
#include "codegen/x86/LivenessAnalysis.hpp"

jl::x86::LiveIntervalMap jl::x86::pass::liveness_analysis(jl::x86::MachineFunction* function)
{
    LivenessAnalysis la(function);
    auto intervals = la.calculate_live_intervals();

    std::println("{}", function->to_str());

    for (const auto& [reg, interval] : intervals) {
        auto already_allocated = function->get_allocation(reg) ? "(already allocated)" : "";
        std::println("{}: [{}, {}] {}", reg.to_str(), interval.start, interval.end, already_allocated);
    }

    return intervals;
}

bool is_an_input_param(jl::x86::MachineFunction* function, const jl::x86::VirtualRegister& vreg)
{
    return std::find(function->inputs().cbegin(), function->inputs().cend(), vreg) != function->inputs().cend();
}

void save_dx_reg_for_div_operations(jl::x86::MachineFunction* function, const std::unordered_set<jl::x86::PhysicalRegister::Type>& allocated_regs)
{
    // TODO::Rather than looking at the all registers allocated throughout the function,
    // save the registers live during the divison operation like we do with call operation
    if (!allocated_regs.contains(jl::x86::PhysicalRegister::rdx)) {
        return;
    }

    for (auto& block : function->blocks()) {
        for (auto iter = block->m_instructions.begin(); iter != block->m_instructions.end(); ++iter) {
            auto call = dynamic_cast<jl::x86::Div*>(iter->get());

            // we only care about idiv
            if (call == nullptr || call->is_float) {
                continue;
            }

            auto rdx = function->new_register();
            function->set_allocation(rdx, jl::x86::PhysicalRegister(jl::x86::PhysicalRegister::rdx));
            auto push = std::make_unique<jl::x86::Push>();
            push->value = rdx;

            auto cqo = std::prev(iter);
            block->m_instructions.insert(cqo, std::move(push));

            auto pop = std::make_unique<jl::x86::Pop>();
            pop->value = rdx;
            block->m_instructions.insert(std::next(iter), std::move(pop));
        }
    }
}

jl::x86::AllocationMap jl::x86::pass::linear_scan_reg_allocation(jl::x86::MachineFunction* function,
    const jl::x86::LiveIntervalMap& intervals,
    uint8_t gpr_count,
    uint8_t float_count)
{
    auto allocator = LinearScanAllocator(function, intervals, gpr_count, float_count);
    const auto allocations = allocator.run();
    jl::x86::AllocationMap allocation_map;

    for (auto& [range, allocation] : allocations) {
        std::println("[{}, {}] -> {}", range.start, range.end, allocation.to_str());
    }

    for (auto [reg, range] : intervals) {
        if (function->get_allocation(reg) && !is_an_input_param(function, reg))
            continue;
        allocation_map[reg] = allocations.at(range);
    }

    for (auto& block : function->blocks()) {
        for (auto iter = block->m_instructions.begin(); iter != block->m_instructions.end(); ++iter) {
            auto call = dynamic_cast<Call*>(iter->get());

            if (call == nullptr) {
                continue;
            }

            const auto& active = allocator.m_active_at_call_sites[call->ret_value];

            std::vector<VirtualRegister> active_regs;
            std::ranges::transform(active, std::back_inserter(active_regs), [&function](const auto preg) {
                const auto reg = PhysicalRegister(preg);
                auto vreg = function->new_register(reg.is_float());
                function->set_allocation(vreg, reg);
                return vreg;
            });

            // Push the active registers
            for (const auto reg : active_regs) {
                auto push = std::make_unique<Push>();
                push->value = reg;
                block->m_instructions.insert(iter, std::move(push));
            }

            // Pop the active registers
            for (const auto reg : active_regs) {
                auto pop = std::make_unique<Pop>();
                pop->value = reg;
                // We will be popping only after the intr that moves the return value from the return register
                auto insert_iter = std::next(std::next(iter));
                block->m_instructions.insert(insert_iter, std::move(pop));
            }

            // Move the input arguments
            int gpr_regs = 0;
            int float_regs = 0;
            for (const auto& reg : call->args) {
                auto input_reg = reg.is_float
                    ? input_float_registers[float_regs++]
                    : input_gpr_registers[gpr_regs++];
                auto input_vreg = function->new_register();
                function->set_allocation(input_vreg, PhysicalRegister(input_reg));

                auto move = std::make_unique<Mov>();
                move->is_float = reg.is_float;
                move->source = reg;
                move->dest = input_vreg;
                block->m_instructions.insert(iter, std::move(move));
            }
        }
    }

    save_dx_reg_for_div_operations(function, allocator.m_allocated_regs);

    return allocation_map;
}
