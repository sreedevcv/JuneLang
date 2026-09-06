#include "LinearScanRegAllocator.hpp"

#include "Instruction.hpp"
#include "codegen/x86/Register.hpp"
#include <algorithm>
#include <array>
#include <iterator>
#include <memory>
#include <set>
#include <unordered_set>

void jl::x86::LinearScanAllocator::expire_old_intervals(
    jl::x86::Range new_range,
    std::set<jl::x86::Range, jl::x86::RangeCompare>& active,
    std::unordered_set<jl::x86::PhysicalRegister::Type>& free)
{
    for (auto it = active.begin(); it != active.end();) {
        if (it->end > new_range.start) {
            break;
        }

        auto alloc = allocations.at(*it);
        // TODO::should this be an assert instead?
        if (alloc.type != jl::x86::Allocation::SLOT) {
            free.insert(static_cast<jl::x86::PhysicalRegister::Type>(alloc.value));
        }

        it = active.erase(it);
    }
}

uint32_t jl::x86::LinearScanAllocator::calculate_stack_offset(const jl::x86::VirtualRegister& reg)
{
    const auto var = *m_function->get_variable(reg);
    const auto alignment = var.type()->alignment();

    if (m_function->total_stack_space % alignment != 0) {
        m_function->total_stack_space = ((m_function->total_stack_space + alignment - 1) / alignment) * alignment;
    }

    const auto offset = m_function->total_stack_space;
    m_function->total_stack_space += var.type()->size();

    return offset;
}

void jl::x86::LinearScanAllocator::allot_or_spill(jl::x86::Range range,
    const jl::x86::VirtualRegister& reg,
    std::unordered_set<jl::x86::PhysicalRegister::Type>& free,
    std::set<jl::x86::Range, jl::x86::RangeCompare>& active,
    jl::x86::Allocation::Type type,
    uint32_t reg_count)
{
    if (active.size() >= reg_count) {
        auto spill = *active.rbegin();
        auto slot = jl::x86::Allocation {
            .type = jl::x86::Allocation::SLOT,
            .value = calculate_stack_offset(reg),
        };

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
        allocations[range] = jl::x86::Allocation {
            .type = type,
            .value = reg,
        };
        m_allocated_regs.insert(reg);
        free.erase(free.begin());
        active.insert(range);
    }
}

// Vreg is defined at a call site, store the active registers at this point
// so that we can push/pop them later
void jl::x86::LinearScanAllocator::save_active_registers(const jl::x86::VirtualRegister& vreg)
{
    for (const auto& range : gpr_active) {
        m_active_at_call_sites[vreg].push_back(jl::x86::PhysicalRegister::Type(allocations[range].value));
    }
    for (const auto& range : float_active) {
        m_active_at_call_sites[vreg].push_back(jl::x86::PhysicalRegister::Type(allocations[range].value));
    }
}

jl::x86::LinearScanAllocator::LinearScanAllocator(jl::x86::MachineFunction* function,
    const jl::x86::LiveIntervalMap& intervals,
    uint32_t gpr_count,
    uint32_t float_count)
    : m_function(function)
    , m_intervals(intervals)
    , m_gpr_count(gpr_count)
    , m_float_count(float_count)
{
    for (int i = 0; i < gpr_count; i++) {
        free_gprs.insert(gpr_allocatable_regs[i]);
    }

    for (int i = 0; i < float_count; i++) {
        free_floats.insert(float_allocatable_regs[i]);
    }

    // preallocate the input registers
    for (auto param : m_function->inputs()) {
        const auto reg = std::get<jl::x86::PhysicalRegister>(*m_function->get_allocation(param));
        const auto range = intervals.at(param);

        if (param.is_float) {
            free_floats.erase(reg.reg);
            float_active.insert(range);
        } else {
            free_gprs.erase(reg.reg);
            gpr_active.insert(range);
        }

        allocations[range] = jl::x86::Allocation {
            .type = param.is_float ? jl::x86::Allocation::FLOAT : jl::x86::Allocation::GPR,
            .value = reg.reg,
        };
    }

    for (auto& block : function->blocks()) {
        for (auto& instr : block->m_instructions) {
            if (auto call = dynamic_cast<jl::x86::Call*>(instr.get())) {
                m_active_at_call_sites[call->ret_value] = {};
            }
        }
    }
}

std::unordered_map<jl::x86::Range, jl::x86::Allocation, jl::x86::RangeHasher> jl::x86::LinearScanAllocator::run()
{
    // Sort the ranges
    std::vector<std::pair<jl::x86::VirtualRegister, jl::x86::Range>> sorted_ranges(m_intervals.cbegin(), m_intervals.cend());
    std::sort(sorted_ranges.begin(), sorted_ranges.end(),
        [](auto&& a, auto&& b) {
            return a.second.start < b.second.start;
        });

    for (const auto [vreg, range] : sorted_ranges) {
        expire_old_intervals(range, float_active, free_floats);
        expire_old_intervals(range, gpr_active, free_gprs);

        if (m_active_at_call_sites.contains(vreg)) {
            save_active_registers(vreg);
        }

        if (m_function->get_allocation(vreg))
            continue; // Already allocated

        if (vreg.is_float) {
            allot_or_spill(range, vreg, free_floats, float_active, jl::x86::Allocation::FLOAT, m_float_count);
        } else {
            allot_or_spill(range, vreg, free_gprs, gpr_active, jl::x86::Allocation::GPR, m_gpr_count);
        }
    }

    return allocations;
}
