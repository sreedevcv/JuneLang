#include "Passes.hpp"

#include "RegisterAllocator.hpp"
#include <array>
#include <unordered_set>

const std::array<jl::x86::PhysicalRegister::Type, 13> gpr_allocatable_regs = {
    jl::x86::PhysicalRegister::rdi,
    jl::x86::PhysicalRegister::rsi,
    jl::x86::PhysicalRegister::rbx,
    jl::x86::PhysicalRegister::rcx,
    jl::x86::PhysicalRegister::rdx,
    jl::x86::PhysicalRegister::r8,
    jl::x86::PhysicalRegister::r9,
    jl::x86::PhysicalRegister::r10,
    jl::x86::PhysicalRegister::r11,
    jl::x86::PhysicalRegister::r12,
    jl::x86::PhysicalRegister::r13,
    jl::x86::PhysicalRegister::r14,
    jl::x86::PhysicalRegister::r15,
};

void expire_old_intervals(
    jl::Range new_range,
    std::set<jl::Range, jl::RangeCompare>& active,
    std::unordered_set<jl::x86::PhysicalRegister::Type>& free,
    const std::unordered_map<jl::Range, jl::Allocation, jl::RangeHasher>& allocations)
{
    for (auto it = active.begin(); it != active.end();) {
        if (it->end > new_range.start) {
            break;
        }

        auto alloc = allocations.at(*it);
        // TODO::should this be an assert instead?
        if (alloc.type != jl::Allocation::SLOT) {
            free.insert(static_cast<jl::x86::PhysicalRegister::Type>(alloc.value));
        }

        it = active.erase(it);
    }
}

uint32_t calculate_stack_offset(jl::x86::MachineFunction* function, const jl::x86::VirtualRegister& reg)
{
    const auto var = *function->get_variable(reg);
    const auto alignment = var.type()->alignment();

    if (function->total_stack_space % alignment != 0) {
        function->total_stack_space = ((function->total_stack_space + alignment - 1) / alignment) * alignment;
    }

    const auto offset = function->total_stack_space;
    function->total_stack_space += var.type()->size();

    return offset;
}

void allot_or_spill(jl::Range range,
    const jl::x86::VirtualRegister& reg,
    jl::x86::MachineFunction* function,
    std::unordered_map<jl::Range, jl::Allocation, jl::RangeHasher>& allocations,
    std::unordered_set<jl::x86::PhysicalRegister::Type>& free,
    std::set<jl::Range, jl::RangeCompare>& active,
    jl::Allocation::Type type,
    uint32_t reg_count)
{
    if (active.size() == reg_count) {
        auto spill = *active.rbegin();
        auto slot = jl::Allocation {
            .type = jl::Allocation::SLOT,
            .value = calculate_stack_offset(function, reg),
        };
        // m_total_slots++;

        if (spill.end > range.end) {
            allocations[range] = allocations[spill];
            allocations[spill] = slot;
            active.erase(spill);
            active.insert(range);
        } else {
            allocations[range] = slot;
        }
    } else {
        auto reg = *free.begin();
        allocations[range] = jl::Allocation {
            .type = type,
            .value = reg,
        };
        free.erase(free.begin());
        active.insert(range);
    }
}

bool is_an_input_param(jl::x86::MachineFunction* function, const jl::x86::VirtualRegister& vreg)
{
    return std::find(function->inputs().cbegin(), function->inputs().cend(), vreg) != function->inputs().cend();
}

std::unordered_map<jl::Range, jl::Allocation, jl::RangeHasher> run_allocator(jl::x86::MachineFunction* function, const jl::x86::pass::LiveIntervalMap& intervals)
{
    uint32_t gpr_count = 3;
    uint32_t float_count = 5;

    std::unordered_set<jl::x86::PhysicalRegister::Type> free_gprs;
    std::unordered_set<jl::x86::PhysicalRegister::Type> free_floats;
    std::set<jl::Range, jl::RangeCompare> gpr_active;
    std::set<jl::Range, jl::RangeCompare> float_active;
    std::unordered_map<jl::Range, jl::Allocation, jl::RangeHasher> allocations;
    uint32_t stack_slots = 0;

    for (int i = 0; i < gpr_count; i++) {
        free_gprs.insert(gpr_allocatable_regs[i]);
    }

    //  for (int i = 0; i < float_count; i++) {
    //      free_floats.insert(i);
    //  }

    assert(function->inputs().size() <= gpr_allocatable_regs.size());

    for (auto param : function->inputs()) {
        const auto reg = std::get<jl::x86::PhysicalRegister>(*function->get_allocation(param));
        const auto range = intervals.at(param);

        free_gprs.erase(reg.reg);
        gpr_active.insert(range);

        allocations[range] = jl::Allocation {
            .type = jl::Allocation::GPR,
            .value = reg.reg,
        };
    }

    // Sort the ranges
    std::vector<std::pair<jl::x86::VirtualRegister, jl::Range>> sorted_ranges(intervals.cbegin(), intervals.cend());
    std::sort(sorted_ranges.begin(), sorted_ranges.end(),
        [](auto&& a, auto&& b) {
            return a.second.start < b.second.start;
        });

    for (const auto [vreg, range] : sorted_ranges) {
        if (function->get_allocation(vreg))
            continue; // Already allocated

        // if (type::is_float(vreg.type())) {
        // expire_old_intervals(range, float_active, free_floats, allocations);
        // allot_or_spill(range, vreg, allocations, free_floats, float_active, Allocation::FLOAT, m_float_reg_count);
        // } else {
        expire_old_intervals(range, gpr_active, free_gprs, allocations);
        allot_or_spill(range, vreg, function, allocations, free_gprs, gpr_active, jl::Allocation::GPR, gpr_count);
        // }
    }

    return allocations;
}

jl::x86::pass::AllocationMap jl::x86::pass::linear_scan_reg_allocation(jl::x86::MachineFunction* function, const jl::x86::pass::LiveIntervalMap& intervals)
{
    const auto allocations = run_allocator(function, intervals);
    AllocationMap allocation_map;

    for (auto& [range, allocation] : allocations) {
        std::println("[{}, {}] -> {}", range.start, range.end, allocation.to_str());
    }

    for (auto [reg, range] : intervals) {
        if (function->get_allocation(reg) && !is_an_input_param(function, reg))
            continue;
        allocation_map[reg] = allocations.at(range);
    }

    return allocation_map;
}
