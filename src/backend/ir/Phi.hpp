#pragma once

#include "BasicBlock.hpp"
#include "ir/IR.hpp"
#include "types/Type.hpp"
#include "value/Variable.hpp"
#include <cstdint>
#include <functional>
#include <unordered_set>

namespace jl {
namespace ir {

    struct PairPointerHash {
        template <typename T, typename U>
        std::size_t operator()(const std::pair<T*, U*>& p) const
        {
            // Individual pointer hashes using standard library functions
            std::size_t h1 = std::hash<T*> {}(p.first);
            std::size_t h2 = std::hash<U*> {}(p.second);

            // Combine the hashes safely using a bit-mixing constant
            // (Prevents collisions from mirror pairs or aligned heap addresses)
            return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
        }
    };

    struct Phi : public IR {
        Phi(value::Variable dest, value::Variable replacing_addr);

        std::string to_str() const override;

        void accept(IRVisitor& visitor) override;

        bool is_used(value::Variable variable) override;

        std::vector<value::Variable> uses() const override;

        void replace(value::Variable from, value::Variable to) override;

        std::optional<jl::value::Variable> def() override;

        value::Variable m_replacing_addr;
        value::Variable m_dest;

        struct HashType {
            inline void hash_combine(std::size_t& seed, std::size_t value) const
            {
                seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }

            std::size_t operator()(const std::pair<value::Variable, BasicBlock*>& p) const
            {
                auto h1 = std::hash<const BasicBlock*> {}(p.second);
                auto h2 = std::hash<const type::Type*> {}(p.first.type());
                auto h3 = std::hash<uint32_t> {}(p.first.id());

                hash_combine(h1, h2);
                hash_combine(h1, h3);
                return h1;
            }
        };

        struct EqualType {
            bool operator()(const std::pair<value::Variable, BasicBlock*>& lhs,
                const std::pair<value::Variable, BasicBlock*>& rhs) const
            {
                return lhs.second == rhs.second && lhs.first.id() == rhs.first.id() && lhs.first.type() == rhs.first.type();
            }
        };

        std::unordered_set<std::pair<value::Variable, BasicBlock*>, HashType> m_opers;
    };

}
}
