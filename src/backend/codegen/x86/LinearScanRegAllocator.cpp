#include "Passes.hpp"

#include "codegen/x86/Instruction.hpp"
#include "codegen/x86/MachineFunction.hpp"
#include "codegen/x86/Operand.hpp"
#include "codegen/x86/Register.hpp"
#include <algorithm>
#include <cstdint>
#include <iterator>
#include <memory>
#include <optional>
#include <print>
#include <set>
#include <sys/types.h>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

using VRegPair = std::pair<jl::x86::VirtualRegister, jl::x86::pass::LiveInterval>;
using PRegPair = std::pair<jl::x86::PhysicalRegister, jl::x86::pass::LiveInterval>;

struct AllocationPrinter {
    void operator()(const jl::x86::PhysicalRegister& reg) const
    {
        std::print("reg: {}", reg.to_string());
    }

    void operator()(const uint32_t stack_slot) const
    {
        std::print("stack: {}", stack_slot);
    }
};

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
    std::vector<jl::x86::VirtualRegister> stack_slots;
    jl::x86::pass::RegisterAllocationMap allocations;

    LinerarScanRegisterAllocator(jl::x86::MachineFunction* function,
        const jl::x86::pass::LiveIntervalMap& interval_map)
        : function(function)
        , interval_map(interval_map)
    {
        max_registers = static_cast<int>(jl::x86::PhysicalRegister::REG_SENTINEL);
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
            } else if (available.size() > 0 && reg.hint->reg == jl::x86::PhysicalRegister::REG_SENTINEL) {
                // We must allocate any register to this virtual register
                int new_reg = *available.begin();
                available.erase(new_reg);
                phys_reg = jl::x86::PhysicalRegister(static_cast<jl::x86::PhysicalRegister::Type>(new_reg));
            } else {
                // Find and remove the already allocated register
                auto iter = std::find_if(
                    active.begin(),
                    active.end(),
                    [&](auto&& pair) {
                        return pair.first.hint->reg == phys_reg->reg;
                    });

                // If we are looking for a normal register, we will find it here but we might also be looking
                // for REGISTER_MAX(will never be present in active), which indicates we must allocate this
                // virtual register to any physical register, so for that we will select the active register
                // with the longest interval
                auto selected_to_spill = iter == active.end() ? *active.rbegin() : *iter;

                phys_reg = selected_to_spill.first.hint;
                active.erase(selected_to_spill);

                selected_to_spill.first.hint = std::nullopt;
                allocate(selected_to_spill.first, { .start = interval.start, .end = selected_to_spill.second.end });
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
            allocations[spill.first].back().interval.end = interval.start - 1;
            stack_slots.push_back(reg);
            allocations[spill.first].push_back({ .allocation = stack_slot++,
                .interval = {
                    .start = interval.start,
                    .end = spill.second.end } });

            allocation_range = { .allocation = *spill.first.hint,
                .interval = {
                    .start = interval.start,
                    .end = spill.second.end } };
        } else {
            stack_slots.push_back(reg);
            allocation_range = {
                .allocation = stack_slot++,
                .interval = interval
            };
        }

        allocations[reg].push_back(allocation_range);
    }
};

static void change_register_sizes_for_cmp_instrs(jl::x86::MachineFunction* function)
{
    std::vector<jl::x86::VirtualRegister> byte_regs;

    for (auto& block : function->blocks()) {
        for (auto& instr : block->m_instructions) {
            if (auto less = dynamic_cast<jl::x86::Less*>(instr.get())) {
                less->reg.hint->is_byte = true;
                byte_regs.push_back(less->reg);
            } else if (auto equals = dynamic_cast<jl::x86::Equals*>(instr.get())) {
                equals->reg.hint->is_byte = true;
                byte_regs.push_back(equals->reg);
            }
        }
    }

    for (auto& block : function->blocks()) {
        for (auto& instr : block->m_instructions) {
            for (auto reg : byte_regs) {
                instr->replace(reg, reg);
            }
        }
    }
}

static std::vector<jl::x86::MemoryOperand> compute_stack_addresses(
    jl::x86::MachineFunction* function,
    std::vector<jl::x86::VirtualRegister>& stack_slots)
{
    std::vector<jl::x86::MemoryOperand> addresses(stack_slots.size());

    for (int i = 0; i < stack_slots.size(); i++) {
        const auto data_size = function->get_data_size_from_virtual_register(stack_slots[i]);
        const int32_t offset = -(function->total_stack_space + data_size);
        function->total_stack_space += data_size;

        jl::x86::MemoryOperand address;
        address.base = function->get_physical_register(jl::x86::PhysicalRegister::rbp);
        address.displacement = offset;
        address.index = std::nullopt;

        addresses[i] = std::move(address);
    }

    return addresses;
}

static void annotate_instructions(jl::x86::MachineFunction* function,
    jl::x86::pass::RegisterAllocationMap& allocations,
    std::vector<jl::x86::VirtualRegister>& stack_slots)
{
    auto rpo = function->rpo();
    auto stack_address = compute_stack_addresses(function, stack_slots);

    const auto allocation_to_operand = [&](jl::x86::pass::Allocation allocation,
                                           jl::x86::VirtualRegister replacer) -> jl::x86::Operand {
        if (auto reg = std::get_if<jl::x86::PhysicalRegister>(&allocation)) {
            jl::x86::VirtualRegister vreg;
            vreg.id = replacer.id;
            vreg.hint = *reg;

            return vreg;
        } else {
            auto slot = std::get<uint32_t>(allocation);
            return stack_address[slot];
        }
    };

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

        jl::x86::Operand operator()(const jl::x86::pass::StackSlot& slot) const
        {
            const auto data_size = function->get_data_size_from_virtual_register(replacer);
            const int32_t offset = -(function->total_stack_space + data_size);
            function->total_stack_space += data_size;

            jl::x86::MemoryOperand stack_source;
            stack_source.base = function->get_physical_register(jl::x86::PhysicalRegister::rbp);
            stack_source.displacement = offset;
            stack_source.index = std::nullopt;

            return stack_source;
        }
    };

    // Give each vistual register its allocated operand
    for (const auto [reg, ranges] : allocations) {
        for (auto block : rpo) {
            for (int i = 0; i < ranges.size(); i++) {
                const auto& range = ranges[i];

                for (auto it = block->m_instructions.begin(); it != block->m_instructions.end(); ++it) {
                    auto& instr = *it;

                    if (range.interval.contains(instr->m_id)) {
                        // auto operand = std::visit(OperandFromAllocation(reg, function), range.allocation);
                        auto operand = allocation_to_operand(range.allocation, reg);
                        instr->replace(reg, operand);
                    }

                    // Insert a move at the end of the range
                    if (i < ranges.size() - 1 && instr->m_id == range.interval.end) {
                        auto move = new jl::x86::Mov();
                        move->m_id = 1000;
                        // auto operand = allocation_to_operand(range.allocation, reg);
                        // move->dest = std::visit(OperandFromAllocation(reg, function), ranges[i + 1].allocation);
                        // move->source = std::visit(OperandFromAllocation(reg, function), ranges[i].allocation);
                        move->source = allocation_to_operand(ranges[i].allocation, reg);
                        move->dest = allocation_to_operand(ranges[i + 1].allocation, reg);
                        move->is_float = false;

                        auto uses = instr->uses();
                        block->m_instructions.insert(it, std::unique_ptr<jl::x86::Instruction>(move));
                    }
                }
            }
        }

        for (const auto [reg, ranges] : allocations) {
            for (int i = 0; i < ranges.size() - 1; i++) {
                auto source = ranges[i];
                auto dest = ranges[i + 1];
            }
        }
    }

    change_register_sizes_for_cmp_instrs(function);
}

jl::x86::pass::RegisterAllocationMap jl::x86::pass::linear_scan_reg_allocation(jl::x86::MachineFunction* function, const jl::x86::pass::LiveIntervalMap& intervals)
{
    LinerarScanRegisterAllocator allocator(function, intervals);
    allocator.calculate();

    std::println("Allocations: ");
    for (const auto& [reg, ranges] : allocator.allocations) {
        std::print("{}: ", reg.to_string());

        for (const auto& range : ranges) {
            std::visit(AllocationPrinter {}, range.allocation);
            std::print(" [{}, {}],", range.interval.start, range.interval.end);
        }
        std::println();
    }
    annotate_instructions(function, allocator.allocations, allocator.stack_slots);

    const auto replaced = function->to_string();

    std::println("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    std::println("{}", replaced);

    return allocator.allocations;
}
