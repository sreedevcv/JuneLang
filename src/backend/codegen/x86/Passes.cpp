#include "codegen/x86/Passes.hpp"

#include "codegen/x86/Instruction.hpp"
#include "codegen/x86/LinearScanRegAllocator.hpp"
#include "codegen/x86/LivenessAnalysis.hpp"
#include "codegen/x86/MachineFunction.hpp"

#include <print>
#include <variant>

jl::x86::MachineAllocPrinter::MachineAllocPrinter(jl::x86::MachineFunction* function)
    : function(function)
{
}

std::string jl::x86::MachineAllocPrinter::operator()(const jl::x86::PhysicalRegister& reg) const
{
    return reg.to_str();
}

std::string jl::x86::MachineAllocPrinter::operator()(const jl::x86::MemoryOperand& mem) const
{
    auto base_reg = *function->get_allocation(mem.base);
    std::string addr = std::visit(MachineAllocPrinter(function), base_reg);
    auto size_dir = (mem.size ? to_str(*mem.size) : "");

    if (mem.index) {
        auto index_reg = *function->get_allocation(*mem.index);
        auto index_str = std::visit(MachineAllocPrinter(function), index_reg);
        addr += std::to_string(mem.scale) + " * " + index_str;
    }

    if (mem.displacement != 0) {
        addr += std::to_string(mem.displacement);
    }

    return size_dir + "[" + addr + "]";
}

std::string jl::x86::MachineAllocPrinter::operator()(const jl::x86::MemoryLabel& mem) const
{
    std::string s = mem.size != jl::x86::SizeDirective::NONE
        ? to_str(mem.size)
        : "";
    return s + "[" + mem.label + "]";
}

std::string jl::x86::MachineAllocPrinter::operator()(const int64_t& imm) const
{
    return std::to_string(imm);
}

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

            auto rdx = function->new_register(jl::x86::SizeDirective::QWORD);
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

jl::x86::AllocationResult jl::x86::pass::linear_scan_reg_allocation(jl::x86::MachineFunction* function,
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

    // Move function inputs to registers before a call
    save_dx_reg_for_div_operations(function, allocator.m_allocated_regs);

    return {
        .active_at_call_sites = std::move(allocator.m_active_at_call_sites),
        .allocations = allocation_map
    };
}
