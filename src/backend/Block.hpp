#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <unordered_map>

#include "backend/value/Variable.hpp"

namespace jl {
class Block {
public:
    struct VarData {
        uint32_t temp_var_count { 0 };
        uint32_t stack_var_count { 0 };
        uint32_t label_count { 0 };
    };

    Block(const Block* parent, VarData* var_data);

    ~Block() = default;

    Block(const Block&) = delete;

    Block& operator=(const Block&) = delete;

    Block(Block&&) = default;

    Block& operator=(Block&&) = default;

    std::shared_ptr<value::Variable> create_varaible(value::Variable::Storage type = value::Variable::TEMP);

    std::shared_ptr<value::Variable> create_named_variable(const std::string& name);

    std::optional<std::shared_ptr<value::Variable>> lookup_variable(const std::string& name) const;

    uint32_t create_label();

protected:
    std::unordered_map<std::string, std::shared_ptr<value::Variable>> m_symbol_table;

    const Block* m_parent;

    VarData* m_var_data;
};

}
