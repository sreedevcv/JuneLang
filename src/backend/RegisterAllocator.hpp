#pragma once

#include "BasicBlock.hpp"
#include "Function.hpp"
#include "ir/IR.hpp"
#include "value/Variable.hpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace jl {

class RegisterAllocator {
public:
    RegisterAllocator(Function* funtion, uint32_t gpr_count, uint32_t float_reg_count);

    using AllocationResult = std::pair<std::unordered_map<value::Variable, Allocation, value::VariableHasher>, uint32_t>;

    AllocationResult allocate();

private:
    uint32_t m_gpr_count;
    uint32_t m_float_reg_count;
    uint32_t m_total_slots;
    uint32_t m_total_offset;
    Function* m_function;
    std::vector<BasicBlock*> rpo;

    using VariableSet = std::unordered_set<value::Variable, value::VariableHasher>;
    using LiveSet = std::unordered_map<BasicBlock*, VariableSet>;
    using Ranges = std::unordered_map<jl::value::Variable, jl::Range, jl::value::VariableHasher>;

    std::unordered_map<ir::IR*, uint32_t> number_instructions() const;

    LiveSet compute_liveness() const;

    Ranges compute_intervals(LiveSet live_in, std::unordered_map<jl::ir::IR*, uint32_t>& numbering) const;

    uint32_t calculate_stack_offset(const value::Variable& var);

    void expire_old_intervals(Range new_range,
        std::set<Range, RangeCompare>& active,
        std::unordered_set<uint32_t>& free,
        const std::unordered_map<Range, Allocation, RangeHasher>& allocation);

    std::pair<std::unordered_map<Range, Allocation, RangeHasher>, uint32_t> linear_allocate(Ranges ranges);

    void allot_or_spill(Range range,
        const value::Variable& var,
        std::unordered_map<Range, Allocation, RangeHasher>& allocations,
        std::unordered_set<uint32_t>& free,
        std::set<Range, RangeCompare>& active,
        Allocation::Type type,
        uint32_t reg_count);
};

}
