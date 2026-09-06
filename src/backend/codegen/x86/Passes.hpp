#pragma once

#include "codegen/x86/MachineFunction.hpp"
#include "codegen/x86/Register.hpp"
#include <cstdint>
#include <unordered_set>

namespace jl {
namespace x86 {
    namespace pass {
        struct Allocation {
            enum Type {
                GPR,
                FLOAT,
                SLOT,
            };

            Type type;
            uint32_t value;

            inline std::string to_str() const
            {
                std::string s;
                switch (type) {
                case GPR:
                    s = "gpr";
                    break;
                case FLOAT:
                    s = "flt";
                    break;
                case SLOT:
                    s = "stk";
                    break;
                }
                s += std::to_string(value);
                return s;
            }
        };

        struct Range {
            int32_t start = -1;
            int32_t end = -1;
            VirtualRegister vreg;

            inline void add_range(int32_t new_start, int32_t new_end)
            {
                start = start == -1 ? new_start : std::min(start, new_start);
                end = end == -1 ? new_end : std::max(end, new_end);
            }

            inline void set_start(int32_t new_start)
            {
                // assert(end == -1 || new_start <= end);
                start = new_start;
            }

            inline bool contains(int32_t point) const
            {
                return point >= start && point <= end;
            }

            inline bool operator==(const Range& other) const
            {
                return start == other.start && end == other.end;
            }
        };

        struct RangeCompare {
            bool operator()(const Range& a, const Range& b) const
            {
                if (a.end != b.end) {
                    return a.end < b.end;
                }
                return a.start < b.start;
            }
        };

        struct RangeHasher {
            std::size_t operator()(const Range& range) const
            {
                return VirtualRegisterHasher {}(range.vreg);
            }
        };

        using LiveIntervalMap = std::unordered_map<jl::x86::VirtualRegister, Range, jl::x86::VirtualRegisterHasher>;
        using reg_set = std::unordered_set<jl::x86::VirtualRegister, jl::x86::VirtualRegisterHasher>;
        using AllocationMap = std::unordered_map<jl::x86::VirtualRegister, Allocation, jl::x86::VirtualRegisterHasher>;

        LiveIntervalMap liveness_analysis(MachineFunction* function);

        AllocationMap linear_scan_reg_allocation(jl::x86::MachineFunction* function, const jl::x86::pass::LiveIntervalMap& intervals);

        void assign_register(MachineFunction* function, AllocationMap allocations);

        struct AssemblyProgram {
            std::string text_section;
            std::string data_section;
        };

        void to_nasm_assembly(AssemblyProgram& program, MachineFunction* function);
    }

    struct MachineAllocPrinter {
        jl::x86::MachineFunction* function;

        MachineAllocPrinter(jl::x86::MachineFunction* function)
            : function(function)
        {
        }

        std::string operator()(const jl::x86::PhysicalRegister& reg) const
        {
            return reg.to_str();
        }

        std::string operator()(const jl::x86::MemoryOperand& mem) const
        {
            auto base_reg = *function->get_allocation(mem.base);
            std::string addr = std::visit(MachineAllocPrinter(function), base_reg);
            auto size_dir = (mem.size ? jl::x86::to_str(*mem.size) : "");

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

        std::string operator()(const jl::x86::MemoryLabel& mem) const
        {
            std::string s = mem.size != jl::x86::SizeDirective::NONE
                ? jl::x86::to_str(mem.size)
                : "";
            return s + "[" + mem.label + "]";
        }

        std::string operator()(const int64_t& imm) const
        {
            return std::to_string(imm);
        }
    };
}
}
