#include "Block.hpp"

#include "backend/value/Variable.hpp"

jl::Block::Block(const Block* parent, value::VarData* var_data)
    : m_parent(parent)
    , m_var_data(var_data)
{
}

jl::value::Variable jl::Block::create_varaible(
    std::string func_name,
    const type::Type* data_type,
    value::Variable::Storage type)
{
    auto idx = m_var_data->add_variable(data_type->size());
    return value::Variable(func_name, idx, type);
}

jl::value::Variable jl::Block::create_named_variable(
    std::string func_name,
    const std::string& name,
    const type::Type* data_type)
{
    auto var = create_varaible(func_name, data_type, value::Variable::STACK);
    m_symbol_table.insert({ name, var });
    return var;
}

std::optional<jl::value::Variable> jl::Block::lookup_variable(const std::string& name) const
{
    const Block* block = this;
    do {
        if (block->m_symbol_table.contains(name)) {
            return block->m_symbol_table.at(name);
        }

        block = block->m_parent;
    } while (block != nullptr);

    return std::nullopt;
}

void jl::Block::set_variable_size(const value::Variable& unsized_var, const value::Variable& sized_var)
{
    m_var_data->reset_offset(unsized_var.id(), sized_var.id());
}

uint32_t jl::Block::create_label()
{
    return m_var_data->new_label();
}

jl::value::Variable jl::Block::allocate_space(std::string func_name, uint32_t size)
{
    const auto idx = m_var_data->add_variable(size);
    return value::Variable(func_name, idx, value::Variable::STACK);
}
