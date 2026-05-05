#pragma once

#include <optional>
#include <unordered_map>

#include "backend/value/Variable.hpp"
#include "types/Type.hpp"

namespace jl {
class Function;

class SemanticBlock {
public:
    SemanticBlock(const SemanticBlock* parent, Function* function);

    ~SemanticBlock() = default;

    SemanticBlock(const SemanticBlock&) = delete;

    SemanticBlock& operator=(const SemanticBlock&) = delete;

    SemanticBlock(SemanticBlock&&) = default;

    SemanticBlock& operator=(SemanticBlock&&) = default;

    value::Variable create_varaible(const type::Type* data_type);

    value::Variable create_named_variable(const std::string& name, const type::Type* pointer);

    std::optional<value::Variable> lookup_variable(const std::string& name) const;

protected:
    std::unordered_map<std::string, value::Variable> m_symbol_table;

    const SemanticBlock* m_parent;

    Function* m_function;
};

}
