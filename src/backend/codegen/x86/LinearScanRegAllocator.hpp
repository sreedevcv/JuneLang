#pragma once

#include "codegen/x86/MachineFunction.hpp"
#include "codegen/x86/Range.hpp"
#include "codegen/x86/Register.hpp"

#include <set>
#include <unordered_map>
#include <unordered_set>

namespace jl {
namespace x86 {

    class LinearScanAllocator {
    public:
        std::unordered_map<VirtualRegister, std::vector<PhysicalRegister::Type>, VirtualRegisterHasher> m_active_at_call_sites;
        std::unordered_set<PhysicalRegister::Type> m_allocated_regs;

        LinearScanAllocator(MachineFunction* function,
            const LiveIntervalMap& intervals,
            uint32_t gpr_count,
            uint32_t float_count);

        std::unordered_map<Range, Allocation, RangeHasher> run();

    private:
        const std::array<PhysicalRegister::Type, 13> gpr_allocatable_regs = {
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

        const std::array<PhysicalRegister::Type, 16> float_allocatable_regs = {
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

        MachineFunction* m_function;
        std::unordered_map<Range, Allocation, RangeHasher> allocations;
        const LiveIntervalMap& m_intervals;

        std::unordered_set<PhysicalRegister::Type> free_gprs;
        std::unordered_set<PhysicalRegister::Type> free_floats;
        std::set<Range, RangeCompare> gpr_active;
        std::set<Range, RangeCompare> float_active;

        uint32_t m_gpr_count = 3;
        uint32_t m_float_count = 1;

        void expire_old_intervals(
            Range new_range,
            std::set<Range, RangeCompare>& active,
            std::unordered_set<jl::x86::PhysicalRegister::Type>& free);

        uint32_t calculate_stack_offset(const jl::x86::VirtualRegister& reg);

        void allot_or_spill(Range range,
            const jl::x86::VirtualRegister& reg,
            std::unordered_set<jl::x86::PhysicalRegister::Type>& free,
            std::set<Range, RangeCompare>& active,
            Allocation::Type type,
            uint32_t reg_count);

        // Vreg is defined at a call site, store the active registers at this point
        // so that we can push/pop them later
        void save_active_registers(const jl::x86::VirtualRegister& vreg);
    };
}
}
