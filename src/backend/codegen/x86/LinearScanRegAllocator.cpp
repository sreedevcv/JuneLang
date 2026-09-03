#include "Passes.hpp"

#include "Instruction.hpp"
#include "RegisterAllocator.hpp"
#include "codegen/x86/Register.hpp"
#include <algorithm>
#include <array>
#include <iterator>
#include <memory>
#include <unordered_set>

class LinearScanAllocator {
private:
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

    const std::array<jl::x86::PhysicalRegister::Type, 16> float_allocatable_regs = {
        jl::x86::PhysicalRegister::xmm0,
        jl::x86::PhysicalRegister::xmm1,
        jl::x86::PhysicalRegister::xmm2,
        jl::x86::PhysicalRegister::xmm3,
        jl::x86::PhysicalRegister::xmm4,
        jl::x86::PhysicalRegister::xmm5,
        jl::x86::PhysicalRegister::xmm6,
        jl::x86::PhysicalRegister::xmm7,
        jl::x86::PhysicalRegister::xmm8,
        jl::x86::PhysicalRegister::xmm9,
        jl::x86::PhysicalRegister::xmm10,
        jl::x86::PhysicalRegister::xmm11,
        jl::x86::PhysicalRegister::xmm12,
        jl::x86::PhysicalRegister::xmm13,
        jl::x86::PhysicalRegister::xmm14,
    };

    jl::x86::MachineFunction* m_function;
    std::unordered_map<jl::Range, jl::Allocation, jl::RangeHasher> allocations;
    const jl::x86::pass::LiveIntervalMap& m_intervals;
    std::unordered_set<jl::x86::PhysicalRegister::Type> m_allocated_regs;

    std::unordered_set<jl::x86::PhysicalRegister::Type> free_gprs;
    std::unordered_set<jl::x86::PhysicalRegister::Type> free_floats;
    std::set<jl::Range, jl::RangeCompare> gpr_active;
    std::set<jl::Range, jl::RangeCompare> float_active;

    uint32_t m_gpr_count = 3;
    uint32_t m_float_count = 1;

    void expire_old_intervals(
        jl::Range new_range,
        std::set<jl::Range, jl::RangeCompare>& active,
        std::unordered_set<jl::x86::PhysicalRegister::Type>& free)
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

    uint32_t calculate_stack_offset(const jl::x86::VirtualRegister& reg)
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

    void allot_or_spill(jl::Range range,
        const jl::x86::VirtualRegister& reg,
        std::unordered_set<jl::x86::PhysicalRegister::Type>& free,
        std::set<jl::Range, jl::RangeCompare>& active,
        jl::Allocation::Type type,
        uint32_t reg_count)
    {
        if (active.size() == reg_count) {
            auto spill = *active.rbegin();
            auto slot = jl::Allocation {
                .type = jl::Allocation::SLOT,
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
            allocations[range] = jl::Allocation {
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
    void save_active_registers(const jl::x86::VirtualRegister& vreg)
    {
        for (const auto& range : gpr_active) {
            m_active_at_call_sites[vreg].push_back(jl::x86::PhysicalRegister::Type(allocations[range].value));
        }
        for (const auto& range : float_active) {
            m_active_at_call_sites[vreg].push_back(jl::x86::PhysicalRegister::Type(allocations[range].value));
        }
    }

public:
    std::unordered_map<jl::x86::VirtualRegister, std::vector<jl::x86::PhysicalRegister::Type>, jl::x86::VirtualRegisterHasher> m_active_at_call_sites;

    LinearScanAllocator(jl::x86::MachineFunction* function,
        const jl::x86::pass::LiveIntervalMap& intervals,
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

            allocations[range] = jl::Allocation {
                .type = param.is_float ? jl::Allocation::FLOAT : jl::Allocation::GPR,
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

    std::unordered_map<jl::Range, jl::Allocation, jl::RangeHasher> run()
    {
        // Sort the ranges
        std::vector<std::pair<jl::x86::VirtualRegister, jl::Range>> sorted_ranges(m_intervals.cbegin(), m_intervals.cend());
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
                allot_or_spill(range, vreg, free_floats, float_active, jl::Allocation::FLOAT, m_float_count);
            } else {
                allot_or_spill(range, vreg, free_gprs, gpr_active, jl::Allocation::GPR, m_gpr_count);
            }
        }

        return allocations;
    }
};

bool is_an_input_param(jl::x86::MachineFunction* function, const jl::x86::VirtualRegister& vreg)
{
    return std::find(function->inputs().cbegin(), function->inputs().cend(), vreg) != function->inputs().cend();
}

jl::x86::pass::AllocationMap jl::x86::pass::linear_scan_reg_allocation(jl::x86::MachineFunction* function, const jl::x86::pass::LiveIntervalMap& intervals)
{
    auto allocator = LinearScanAllocator(function, intervals, 10, 1);
    const auto allocations = allocator.run();
    AllocationMap allocation_map;

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

    return allocation_map;
}
