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

struct Allocation {
    enum Type {
        GPR,
        FLOAT,
        SLOT,
    };

    Type type;
    uint32_t value;

    std::string to_str() const
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

class Range {
public:
    int32_t start = -1;
    int32_t end = -1;

    Range() { }

    inline void add_range(int32_t new_start, int32_t new_end)
    {
        start = start == -1 ? new_start : std::min(start, new_start);
        end = end == -1 ? new_end : std::max(end, new_end);
    }

    inline void set_start(int32_t new_start)
    {
        assert(new_start <= end);
        start = new_start;
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
    //  bool operator()(const jl::Range& a, const jl::Range& b) const
    //  {
    //      return a.end < b.end;
    //  }
};

struct RangeHasher {
    std::size_t operator()(const Range& range) const
    {
        auto hash1 = std::hash<uint32_t> {}(range.start);
        auto hash2 = std::hash<uint32_t> {}(range.end);
        std::size_t seed = hash1;
        seed ^= hash2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};

class RegisterAllocator {
public:
    RegisterAllocator(Function* funtion, uint32_t gpr_count, uint32_t float_reg_count);

    //  struct AllocationResult {
    //      std::unordered_map<Range, Allocation, RangeHasher> allocations;
    //      uint32_t stack_slot_count;
    //      std::unordered_map<jl::value::Variable, jl::Range, jl::value::VariableHasher> ranges;

    using AllocationResult = std::pair<std::unordered_map<value::Variable, Allocation, value::VariableHasher>, uint32_t>;
    AllocationResult allocate();

private:
    uint32_t m_gpr_count;
    uint32_t m_float_reg_count;
    uint32_t m_total_slots;
    Function* m_function;
    std::vector<BasicBlock*> rpo;

    using VariableSet = std::unordered_set<value::Variable, value::VariableHasher>;
    using LiveSet = std::unordered_map<BasicBlock*, VariableSet>;
    using Ranges = std::unordered_map<jl::value::Variable, jl::Range, jl::value::VariableHasher>;

    std::unordered_map<ir::IR*, uint32_t> number_instructions() const;

    LiveSet compute_liveness() const;

    Ranges compute_intervals(LiveSet live_in, std::unordered_map<jl::ir::IR*, uint32_t>& numbering) const;

    void expire_old_intervals(Range new_range,
        std::set<Range, RangeCompare>& active,
        std::unordered_set<uint32_t>& free,
        const std::unordered_map<Range, Allocation, RangeHasher>& allocation);

    std::pair<std::unordered_map<Range, Allocation, RangeHasher>, uint32_t> linear_allocate(Ranges ranges);

    void allot_or_spill(Range range,
        std::unordered_map<Range, Allocation, RangeHasher>& allocations,
        std::unordered_set<uint32_t>& free,
        std::set<Range, RangeCompare>& active,
        Allocation::Type type,
        uint32_t reg_count);
};

}
