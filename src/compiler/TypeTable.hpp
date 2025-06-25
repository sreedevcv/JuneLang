#pragma once

#include "Operand.hpp"

#include <array>
#include <cstdint>
#include <unordered_set>

namespace jl {

class TypeTable {
public:
    TypeTable();

private:
    static constexpr uint32_t max_types { 6 };
    std::array<std::array<OperandType, max_types>, max_types> m_arithametic_type_table;
    std::unordered_set<uint32_t> m_alloted_arithametic_rules;
    void add_arithametic_rule(const OperandType t1, const OperandType t2, const OperandType result);

    std::array<std::array<OperandType, max_types>, max_types> m_boolean_type_table;
    std::unordered_set<uint32_t> m_alloted_boolean_rules;
    void add_boolean_rule(const OperandType t1, const OperandType t2, const OperandType result);
};

}