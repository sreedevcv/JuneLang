#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <unordered_map>

#include "backend/value/Variable.hpp"

namespace jl {
class Block {
public:
    Block(const Block* parent, uint32_t id);

    ~Block() = default;

    Block(const Block&) = delete;

    Block& operator=(const Block&) = delete;

    Block(Block&&) = default;

    Block& operator=(Block&&) = default;

    Block* create_block(uint32_t id);

    std::shared_ptr<value::Variable> create_varaible(value::Variable::Storage type = value::Variable::TEMP);

    std::shared_ptr<value::Variable> create_named_variable(const std::string& name);

    std::optional<std::shared_ptr<value::Variable>> lookup_variable(const std::string& name) const;

protected:
    std::unordered_map<std::string, std::shared_ptr<value::Variable>> m_symbol_table;

    uint32_t m_var_count { 0 };
    const Block* m_parent;
    uint32_t m_id;
};

}
