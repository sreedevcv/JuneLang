#include "RegisterAllocator.hpp"
#include "BasicBlock.hpp"
#include "types/Type.hpp"
#include "utils/algorithms.hpp"
#include "value/Variable.hpp"
#include <cstdint>
#include <print>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

jl::RegisterAllocator::RegisterAllocator(Function* funtion, uint32_t gpr_count, uint32_t float_reg_count)
    : m_function(funtion)
    , m_gpr_count(gpr_count)
    , m_float_reg_count(float_reg_count)
{
    auto rpo_map = jl::algorithms::RPO(m_function->entry_block());
    rpo = std::vector<jl::BasicBlock*>(rpo_map.size(), nullptr);

    for (auto& [block, idx] : rpo_map) {
        rpo[idx] = block;
    }
}

std::unordered_map<jl::ir::IR*, uint32_t> jl::RegisterAllocator::number_instructions() const
{
    std::unordered_map<jl::ir::IR*, uint32_t> numbering;
    uint32_t count = 2;

    for (auto& block : rpo) {
        for (auto instr = block->head; instr != nullptr; instr = instr->next) {
            numbering[instr] = count;
            count += 2;
        }
    }

    return numbering;
}

jl::RegisterAllocator::LiveSet jl::RegisterAllocator::compute_liveness() const
{
    // std::unordered_map<BasicBlock*, VariableSet> gen;
    // std::unordered_map<BasicBlock*, VariableSet> kill;
    //
    // // In normal post order traversal
    // for (auto iter = rpo.crbegin(); iter != rpo.crend(); ++iter) {
    // auto block = *iter;
    //
    // VariableSet killed_sofar;
    // for (auto instr = block->tail; instr != nullptr; instr = instr->prev) {
    // for (auto use : instr->uses()) {
    // if (!killed_sofar.contains(use)) {
    // gen[block].insert(use);
    // }
    // }
    // if (auto def = instr->def()) {
    // killed_sofar.insert(*def);
    // kill[block].insert(*def);
    // }
    // }
    // //      for (auto instr = block->tail; instr != nullptr; instr = instr->prev) {
    // //          if (auto def = instr->def()) {
    // //              kill[block].insert(*def);
    // //          }
    //
    // //          for (auto use : instr->uses()) {
    // //              gen[block].insert(use);
    // //          }
    // //      }
    // }
    //
    // LiveSet live_in;
    // bool changed = true;
    //
    // while (changed) {
    // changed = false;
    //
    // for (auto block : rpo) {
    // const auto old_size = live_in[block].size();
    // auto [left, right] = algorithms::get_sucessors(block);
    // VariableSet live_out;
    // if (left != nullptr) {
    // live_out = live_in[left];
    // }
    // if (right != nullptr) {
    // live_out.insert(live_in[right].begin(), live_in[right].end());
    // }
    //
    // VariableSet new_live_in = gen[block];
    // for (auto var : live_out) {
    // if (!kill[block].contains(var)) {
    // new_live_in.insert(var);
    // }
    // }
    //
    // if (new_live_in.size() != old_size) {
    // changed = true;
    // live_in[block] = new_live_in;
    // }
    // }
    // }
    //
    // return live_in;
    LiveSet live_in;
    bool changed = true;
    while (changed) {
        changed = false;
        // For backward dataflow (liveness), iterating in reverse postorder
        // of the CFG converges faster, but any order works.
        for (auto block : rpo) {
            auto old = live_in[block];
            // OUT[B] = union of IN[S] for successors S
            VariableSet live;
            auto [left, right] = algorithms::get_successors(block);
            if (left != nullptr) {
                live.insert(live_in[left].begin(), live_in[left].end());
            }
            if (right != nullptr) {
                live.insert(live_in[right].begin(), live_in[right].end());
            }
            // Walk the block backwards.
            // live_before(instr) = uses(instr) ∪ (live_after(instr) - def(instr))
            for (auto instr = block->tail; instr != nullptr; instr = instr->prev) {
                if (auto def = instr->def()) {
                    live.erase(*def);
                }
                for (auto use : instr->uses()) {
                    live.insert(use);
                }
            }
            if (live != old) {
                changed = true;
                live_in[block] = std::move(live);
            }
        }
    }
    return live_in;
}

jl::RegisterAllocator::Ranges jl::RegisterAllocator::compute_intervals(
    LiveSet live_in,
    std::unordered_map<jl::ir::IR*, uint32_t>& numbering) const
{
    Ranges ranges;

    for (auto input_param : m_function->args()) {
        ranges[input_param].start = 0;
        ranges[input_param].end = 0;
    }

    for (auto iter = rpo.crbegin(); iter != rpo.crend(); ++iter) {
        auto block = *iter;
        auto [left, right] = algorithms::get_successors(block);

        VariableSet live;
        if (left != nullptr) {
            live = live_in[left];
        }
        if (right != nullptr) {
            live.insert(live_in[right].begin(), live_in[right].end());
        }

        for (auto var : live) {
            ranges[var].add_range(numbering[block->head], numbering[block->tail]);
        }

        for (auto ir = block->tail; ir != nullptr; ir = ir->prev) {
            if (auto def = ir->def()) {
                ranges[*def].set_start(numbering[ir]);
            }

            for (auto use : ir->uses()) {
                ranges[use].add_range(numbering[block->head], numbering[ir]);
            }
        }
    }

    return ranges;
}

void jl::RegisterAllocator::expire_old_intervals(
    Range new_range,
    std::set<Range, RangeCompare>& active,
    std::unordered_set<uint32_t>& free,
    const std::unordered_map<Range, Allocation, RangeHasher>& allocations)
{
    for (auto it = active.begin(); it != active.end();) {
        if (it->end > new_range.start) {
            break;
        }

        auto alloc = allocations.at(*it);
        // TODO::should this be an assert instead?
        if (alloc.type != Allocation::SLOT) {
            free.insert(alloc.value);
        }

        it = active.erase(it);
    }
}

bool is_float(const jl::value::Variable& var)
{
    if (auto type = dynamic_cast<const jl::type::Builtin*>(var.type())) {
        return type->m_primitive == jl::type::Builtin::FLOAT;
    }

    return false;
}

void jl::RegisterAllocator::allot_or_spill(Range range,
    std::unordered_map<Range, Allocation, RangeHasher>& allocations,
    std::unordered_set<uint32_t>& free,
    std::set<jl::Range, jl::RangeCompare>& active,
    jl::Allocation::Type type,
    uint32_t reg_count)
{
    if (active.size() == reg_count) {
        auto spill = *active.rbegin();
        auto slot = Allocation {
            .type = Allocation::SLOT,
            .value = m_total_slots++
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
        allocations[range] = Allocation {
            .type = type,
            .value = reg,
        };
        free.erase(free.begin());
        active.insert(range);
    }
}

std::pair<std::unordered_map<jl::Range, jl::Allocation, jl::RangeHasher>, uint32_t>
jl::RegisterAllocator::linear_allocate(Ranges ranges)
{
    assert(m_gpr_count > 0 && m_float_reg_count > 0);

    std::unordered_set<uint32_t> free_gprs;
    std::unordered_set<uint32_t> free_floats;
    std::set<Range, RangeCompare> gpr_active;
    std::set<Range, RangeCompare> float_active;
    std::unordered_map<Range, Allocation, RangeHasher> allocations;
    uint32_t stack_slots = 0;

    for (int i = 0; i < m_gpr_count; i++) {
        free_gprs.insert(i);
    }
    for (int i = 0; i < m_float_reg_count; i++) {
        free_floats.insert(i);
    }

    // Sort the ranges
    std::vector<std::pair<value::Variable, Range>> sorted_ranges(ranges.cbegin(), ranges.cend());
    std::sort(sorted_ranges.begin(), sorted_ranges.end(),
        [](auto&& a, auto&& b) {
            return a.second.start < b.second.start;
        });

    for (const auto [var, range] : sorted_ranges) {
        if (is_float(var)) {
            expire_old_intervals(range, float_active, free_floats, allocations);
            allot_or_spill(range, allocations, free_floats, float_active, Allocation::FLOAT, m_float_reg_count);
        } else {
            expire_old_intervals(range, gpr_active, free_gprs, allocations);
            allot_or_spill(range, allocations, free_gprs, gpr_active, Allocation::GPR, m_gpr_count);
        }
    }

    return { allocations, m_total_slots };
}

jl::RegisterAllocator::AllocationResult jl::RegisterAllocator::allocate()
{
    auto numbering = number_instructions();
    auto liveness = compute_liveness();
    auto ranges = compute_intervals(liveness, numbering);

    std::println("--------------------Ordering------------------");
    for (auto block : rpo) {
        std::println("{}: ", block->get_name());
        for (auto ir = block->head; ir != nullptr; ir = ir->next) {
            std::println("\t[{}]: {}", numbering[ir], ir->to_str());
        }
    }
    std::println("--------------------Liveness------------------");
    for (auto [block, live_in] : liveness) {
        std::print("{}: ", block->get_name());
        for (auto var : live_in) {
            std::print("{}, ", var.to_str());
        }
        std::println();
    }
    std::println("--------------------Ranges------------------");
    for (auto [var, range] : ranges) {
        std::println("{}: {} -> {}", var.to_str(), range.start, range.end);
    }

    auto [allocation, slot_count] = linear_allocate(ranges);

    std::println("--------------------Range Alocation------------------");

    for (auto [range, alloc] : allocation) {
        std::println("{} -> {}", range.start, range.end, alloc.to_str());
    }

    std::unordered_map<value::Variable, Allocation, value::VariableHasher> register_map;
    for (auto [var, range] : ranges) {
        register_map.insert({ var, allocation[range] });
    }

    return { register_map, slot_count };
}
