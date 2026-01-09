#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <unordered_map>

#include "backend/value/Variable.hpp"
#include "types/Type.hpp"

namespace jl {
class Block {
public:
    Block(const Block* parent, value::VarData* var_data);

    ~Block() = default;

    Block(const Block&) = delete;

    Block& operator=(const Block&) = delete;

    Block(Block&&) = default;

    Block& operator=(Block&&) = default;

    std::shared_ptr<value::Variable> create_varaible(
        std::string func_name,
        const type::Type* data_type,
        value::Variable::Storage type = value::Variable::TEMP);

    std::shared_ptr<value::Variable> create_named_variable(
        std::string func_name,
        const std::string& name,
        const type::Type* data_type);

    void set_variable_size(const value::Variable* unsized_var, const value::Variable* sized_var);

    std::optional<std::shared_ptr<value::Variable>> lookup_variable(const std::string& name) const;

    uint32_t create_label();

protected:
    std::unordered_map<std::string, std::shared_ptr<value::Variable>> m_symbol_table;

    const Block* m_parent;

    value::VarData* m_var_data;
};

}
