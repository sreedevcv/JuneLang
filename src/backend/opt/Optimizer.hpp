#pragma once

#include "Function.hpp"
#include "LiteralValue.hpp"

namespace jl {
namespace opt {
    enum LatticeType {
        TOP,
        CONSTANT,
        BOTTOM,
    };

    struct LatticeValue {
        LatticeType type;
        std::optional<jl::LiteralValue> value;

        LatticeValue meet(const LatticeValue& other) const;
        std::string to_str() const;
    };

    using ValueMap = std::unordered_map<jl::value::Variable, LatticeValue, jl::value::VariableHasher>;
    using CFGEdge = std::pair<jl::BasicBlock*, jl::BasicBlock*>;

    struct CFGEdgeHasher {
        std::size_t operator()(const CFGEdge& edge) const
        {
            auto hash1 = std::hash<const jl::BasicBlock*> {}(edge.first);
            auto hash2 = std::hash<const jl::BasicBlock*> {}(edge.second);
            std::size_t seed = hash1;
            seed ^= hash2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            return seed;
        }
    };

    using ExecMap = std::unordered_map<CFGEdge, bool, CFGEdgeHasher>;

    void mem2reg(Function* function);
    std::pair<ValueMap, ExecMap> sccp(Function* function);
    void dce(Function* function, const ValueMap& lattice_values, const ExecMap& exec_map);
    void remove_phi_nodes(jl::Function* function);
}
}
