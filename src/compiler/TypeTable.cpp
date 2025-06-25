#include "TypeTable.hpp"
#include "Operand.hpp"
#include <cstdint>

jl::TypeTable::TypeTable()
{
    add_arithametic_rule(OperandType::INT, OperandType::INT, OperandType::INT);
    add_arithametic_rule(OperandType::INT, OperandType::FLOAT, OperandType::FLOAT);
}

void jl::TypeTable::add_arithametic_rule(const OperandType t1, const OperandType t2, const OperandType result)
{
    const auto i1 = static_cast<uint32_t>(t1);
    const auto i2 = static_cast<uint32_t>(t2);
    m_arithametic_type_table[i1][i2] = result;
    m_arithametic_type_table[i2][i1] = result;
    m_alloted_arithametic_rules.insert((i1 * max_types) + i2);
    m_alloted_arithametic_rules.insert((i2 * max_types) + i1);
}