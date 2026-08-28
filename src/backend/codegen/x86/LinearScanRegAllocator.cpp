#include "Passes.hpp"

#include <unordered_set>
#include "RegisterAllocator.hpp"


// std::unordered_map<jl::x86::Instruction*, uint32_t> number_instructions(const std::vector<jl::x86::MachineBlock*>& rpo) 
// {
    // std::unordered_map<jl::x86::Instruction*, uint32_t> numbering;
    // uint32_t count = 2;
// 
    // for (auto block : rpo) {
        // for (auto& instr: block->m_instructions) {
            // numbering[instr.get()] = count;
            // count += 2;
        // }
    // }
// 
    // return numbering;
// }

// using LiveRanges = std::unordered_map<jl::x86::VirtualRegister, jl::Range, jl::x86::VirtualRegisterHasher>;
// 
// jl::RegisterAllocator::Ranges compute_intervals(
    // MachineFunction* function,
    // const LiveIntervalMap& intervals,
    // const std::vector<jl::x86::MachineBlock*>& rpo,
    // std::unordered_map<jl::x86::Instruction*, uint32_t>& numbering) const
// {
    // LiveRanges ranges;
// 
    // for (auto input_param : function->inputs()) {
        // ranges[input_param].start = 0;
        // ranges[input_param].end = 0;
    // }
// 
    // const auto successors = function->successors();
// 
    // for (auto iter = rpo.crbegin(); iter != rpo.crend(); ++iter) {
        // auto block = *iter;
        // auto succs = successors[block];
// 
        // jl::x86::pass::reg_set live;
        // for (auto succ: succs) {
            // live.insert(succs
        // }
// 
        // VariableSet live;
        // if (left != nullptr) {
            // live = live_in[left];
        // }
        // if (right != nullptr) {
            // live.insert(live_in[right].begin(), live_in[right].end());
        // }
// 
        // for (auto var : live) {
            // ranges[var].add_range(numbering[block->head], numbering[block->tail]);
        // }
// 
        // for (auto ir = block->tail; ir != nullptr; ir = ir->prev) {
            // if (auto def = ir->def()) {
                // ranges[*def].set_start(numbering[ir]);
            // }
// 
            // for (auto use : ir->uses()) {
                // ranges[use].add_range(numbering[block->head], numbering[ir]);
            // }
        // }
    // }
// 
    // return ranges;
// }


void expire_old_intervals(
    jl::Range new_range,
    std::set<jl::Range, jl::RangeCompare>& active,
    std::unordered_set<uint32_t>& free,
    const std::unordered_map<jl::Range, jl::Allocation, jl::RangeHasher>& allocations)
{
    for (auto it = active.begin(); it != active.end();) {
        if (it->end > new_range.start) {
            break;
        }

        auto alloc = allocations.at(*it);
        // TODO::should this be an assert instead?
        if (alloc.type != jl::Allocation::SLOT) {
            free.insert(alloc.value);
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
    std::unordered_set<uint32_t>& free,
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

std::unordered_map<jl::Range, jl::Allocation, jl::RangeHasher> run_allocator(jl::x86::MachineFunction* function, const jl::x86::pass::LiveIntervalMap& intervals)
{
    uint32_t gpr_count = 5;
    uint32_t float_count = 5;
    
    std::unordered_set<uint32_t> free_gprs;
    std::unordered_set<uint32_t> free_floats;
    std::set<jl::Range, jl::RangeCompare> gpr_active;
    std::set<jl::Range, jl::RangeCompare> float_active;
    std::unordered_map<jl::Range, jl::Allocation, jl::RangeHasher> allocations;
    uint32_t stack_slots = 0;

    for (int i = 0; i < gpr_count; i++) {
        free_gprs.insert(i);
    }
    for (int i = 0; i < float_count; i++) {
        free_floats.insert(i);
    }
    
    // Sort the ranges
    std::vector<std::pair<jl::x86::VirtualRegister, jl::Range>> sorted_ranges(intervals.cbegin(), intervals.cend());
    std::sort(sorted_ranges.begin(), sorted_ranges.end(),
        [](auto&& a, auto&& b) {
            return a.second.start < b.second.start;
        });

    for (const auto [vreg, range] : sorted_ranges) {
        if (function->get_allocation(vreg)) continue; // Already allocated

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
    
    for (auto& [range, allocation]: allocations) {
        std::println("[{}, {}] -> {}", range.start, range.end, allocation.to_str());
    }
    
    for (auto [reg, range]: intervals) {
        if (function->get_allocation(reg)) continue;
        allocation_map[reg] = allocations.at(range);
    }
    
    return allocation_map;
}