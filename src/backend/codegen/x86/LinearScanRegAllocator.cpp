#include "Passes.hpp"

#include "codegen/x86/Instruction.hpp"
#include "codegen/x86/MachineFunction.hpp"
#include "codegen/x86/Operand.hpp"
#include "codegen/x86/Register.hpp"
#include <algorithm>
#include <cstdint>
#include <optional>
#include <set>
#include <unordered_set>
#include <variant>

using VRegPair = std::pair<jl::x86::VirtualRegister, jl::x86::pass::LiveInterval>;
using PRegPair = std::pair<jl::x86::PhysicalRegister, jl::x86::pass::LiveInterval>;

struct IntervalCompare {
    bool operator()(const VRegPair& a, const VRegPair& b) const
    {
        return a.second.start < b.second.start;
    }
};

struct ActiveCompare {
    bool operator()(const VRegPair& a, const VRegPair& b) const
    {
        return a.second.end < b.second.end;
    }
};

struct LinerarScanRegisterAllocator {
    jl::x86::MachineFunction* function;
    const jl::x86::pass::LiveIntervalMap& interval_map;

    int max_registers;
    uint32_t stack_slot = 0;
    std::vector<VRegPair> intervals;
    std::unordered_set<int> available;
    std::set<VRegPair, ActiveCompare> active;
    std::vector<int> locations;
    jl::x86::pass::RegisterAllocationMap allocations;

    LinerarScanRegisterAllocator(jl::x86::MachineFunction* function,
        const jl::x86::pass::LiveIntervalMap& interval_map)
        : function(function)
        , interval_map(interval_map)
    {
        max_registers = static_cast<int>(jl::x86::PhysicalRegister::REGISTER_MAX);
        //        max_registers = 3;

        for (auto& [reg, interval] : interval_map) {
            intervals.push_back(std::pair { reg, interval });
        }

        std::sort(intervals.begin(), intervals.end(), IntervalCompare {});

        for (auto i = 0; i < max_registers; i++) {
            available.insert(i);
        }
    }

    void calculate()
    {
        for (auto& pair : intervals) {
            allocate(pair.first, pair.second);
        }
    }

private:
    void allocate(jl::x86::VirtualRegister reg, jl::x86::pass::LiveInterval interval)
    {
        expire_old_intervals(reg, interval);

        if (active.size() == max_registers) {
            spill_at_interval(reg, interval);
        } else {
            //// Extract a register from available
            // int new_reg = *available.begin();
            // available.erase(new_reg);
            //
            // auto phys_reg = jl::x86::PhysicalRegister(static_cast<jl::x86::PhysicalRegister::Type>(new_reg));
            // reg.hint = phys_reg;
            //
            //// Store it in allocations
            // allocations[reg].push_back(jl::x86::pass::AllocationRange {
            //.allocation = phys_reg,
            //.interval = interval,
            //});
            //
            //// Insert the register and interval into active
            // active.insert({ reg, interval });

            allocate_or_spill(reg, interval);
        }
    }

    void allocate_or_spill(jl::x86::VirtualRegister reg, jl::x86::pass::LiveInterval interval)
    {
        jl::x86::PhysicalRegister physical_register;

        if (auto phys_reg = reg.hint) {
            if (available.contains(phys_reg->reg)) {
                // Extract a register from available
                available.erase(phys_reg->reg);
            } else {
                // Find and remove the already allocated register
                auto already_allocated = *std::find_if(
                    active.begin(),
                    active.end(),
                    [&](auto&& pair) {
                        return pair.first.hint->reg == phys_reg->reg;
                    });
                active.erase(already_allocated);

                already_allocated.first.hint = std::nullopt;
                allocate(already_allocated.first, { .start = interval.start, .end = already_allocated.second.end });
            }

            physical_register = *phys_reg;
        } else {
            int new_reg = *available.begin();
            available.erase(new_reg);
            physical_register = jl::x86::PhysicalRegister(static_cast<jl::x86::PhysicalRegister::Type>(new_reg));
        }

        allocations[reg].push_back(jl::x86::pass::AllocationRange {
            .allocation = physical_register,
            .interval = interval,
        });

        reg.hint = physical_register;
        // Insert the register and interval into active
        active.insert({ reg, interval });
    }

    void expire_old_intervals(const jl::x86::VirtualRegister& reg, const jl::x86::pass::LiveInterval& interval)
    {
        std::vector<VRegPair> to_be_removed;

        for (const auto pair : active) {
            if (pair.second.end > interval.start) {
                // All of the active intervals finishes after this starts
                return;
            }

            to_be_removed.push_back(pair);
        }

        for (auto pair : to_be_removed) {
            active.erase(pair);
            available.insert(static_cast<int>(pair.first.hint->reg));
        }
    }

    void spill_at_interval(const jl::x86::VirtualRegister& reg, const jl::x86::pass::LiveInterval& interval)
    {
        jl::x86::pass::AllocationRange allocation_range;

        auto spill = *active.rbegin();
        if (spill.second.end > interval.end) {
            // We will spill the last element of active and use its register for allocating this variable
            active.erase(spill);
            auto new_reg = reg;
            new_reg.hint = spill.first.hint;
            active.insert({ new_reg, interval });

            // Allocate a stack_slot for the spilled register for the rest of its interval
            allocations[spill.first].back().interval.end -= interval.start - 1;
            allocations[spill.first].push_back({ .allocation = stack_slot++,
                .interval = {
                    .start = interval.start,
                    .end = spill.second.end } });

            allocation_range = { .allocation = *spill.first.hint,
                .interval = {
                    .start = interval.start,
                    .end = spill.second.end } };
        } else {
            locations.push_back(reg.id);
            allocation_range = {
                .allocation = stack_slot++,
                .interval = interval
            };
        }

        allocations[reg].push_back(allocation_range);
    }
};

static void annotate_instructions(jl::x86::MachineFunction* function,
    jl::x86::pass::RegisterAllocationMap& allocations)
{
    auto rpo = function->rpo();

    struct OperandFromAllocation {
        const jl::x86::VirtualRegister& replacer;
        jl::x86::MachineFunction* function;

        OperandFromAllocation(const jl::x86::VirtualRegister& replacer,
            jl::x86::MachineFunction* function)
            : replacer(replacer)
            , function(function)
        {
        }

        jl::x86::Operand operator()(const jl::x86::PhysicalRegister& reg) const
        {
            jl::x86::VirtualRegister vreg;
            vreg.id = replacer.id;
            vreg.hint = reg;

            return vreg;
        }

        jl::x86::Operand operator()(const jl::x86::pass::StackSlot& reg) const
        {
            const auto data_size = function->get_data_size_from_virtual_register(replacer);
            const auto offset = -(function->total_stack_space + data_size);
            function->total_stack_space += data_size;

            jl::x86::MemoryOperand stack_source;
            stack_source.base = function->get_physical_register(jl::x86::PhysicalRegister::rbp);
            stack_source.displacement = offset;
            stack_source.index = std::nullopt;

            return stack_source;
        }
    };

    for (const auto [reg, ranges] : allocations) {
        for (auto block : rpo) {
            for (auto& instr : block->m_instructions) {
                // const auto defs = instr->defs();
                // const auto uses = instr->uses();
                for (const auto& range : ranges) {
                    if (range.interval.contains(instr->m_id)) {
                        auto operand = std::visit(OperandFromAllocation(reg, function), range.allocation);
                        instr->replace(reg, operand);
                    }
                }
            }
        }
    }
}

jl::x86::pass::RegisterAllocationMap jl::x86::pass::linear_scan_reg_allocation(jl::x86::MachineFunction* function, const jl::x86::pass::LiveIntervalMap& intervals)
{
    LinerarScanRegisterAllocator allocator(function, intervals);
    allocator.calculate();

    struct AllocationPrinter {
        void operator()(const PhysicalRegister& reg) const
        {
            std::print("reg: {}", reg.to_string());
        }

        void operator()(const uint32_t stack_slot) const
        {
            std::print("stack: {}", stack_slot);
        }
    };

    std::println("Allocations: ");
    for (const auto& [reg, ranges] : allocator.allocations) {
        std::print("{}: ", reg.to_string());

        for (const auto& range : ranges) {
            std::visit(AllocationPrinter {}, range.allocation);
            std::print(" [{}, {}],", range.interval.start, range.interval.end);
        }
        std::println();
    }
    annotate_instructions(function, allocator.allocations);

    const auto replaced = function->to_string();

    std::println("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    std::println("{}", replaced);

    return allocator.allocations;
}
