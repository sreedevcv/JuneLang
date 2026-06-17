#include "SemanticBlock.hpp"

#include "backend/value/Variable.hpp"
#include "Function.hpp"

jl::SemanticBlock::SemanticBlock(const SemanticBlock* parent, Function* function)
    : m_parent(parent)
    , m_function(function)
{
}

jl::value::Variable jl::SemanticBlock::create_varaible(const type::Type* data_type)
{
    return value::Variable(m_function->m_var_count++, data_type);
}

jl::value::Variable jl::SemanticBlock::create_named_variable(const std::string& name, const type::Type* pointer)
{
    // Create a variable to store the stack address
    auto var = create_varaible(pointer);
    // Insert into symbol table
    m_symbol_table.insert({ name, var });
    return var;
}

std::optional<jl::value::Variable> jl::SemanticBlock::lookup_variable(const std::string& name) const
{
    const SemanticBlock* block = this;
    do {
        if (block->m_symbol_table.contains(name)) {
            return block->m_symbol_table.at(name);
        }

        block = block->m_parent;
    } while (block != nullptr);

    return std::nullopt;
}

// void jl::SemanticBlock::set_variable_size(const value::Variable& unsized_var, const value::Variable& sized_var)
// {
//     m_var_data->reset_offset(unsized_var.id(), sized_var.id());
// }

// uint32_t jl::SemanticBlock::create_label()
// {
//     return m_var_data->new_label();
// }

// jl::value::Variable jl::SemanticBlock::allocate_space(uint32_t size, const type::Type* data_type)
// {
//     // const auto idx = m_var_data->add_variable(size);
//     return value::Variable(idx, data_type);
// }
