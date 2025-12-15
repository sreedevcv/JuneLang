#include "Block.hpp"

#include "backend/value/Variable.hpp"
#include <memory>

jl::Block::Block(const Block* parent, VarData* var_data)
    : m_parent(parent)
    , m_var_data(var_data)
{
}

std::shared_ptr<jl::value::Variable> jl::Block::create_varaible(value::Variable::Storage type)
{
    return std::make_shared<value::Variable>(m_var_data->temp_var_count++, type);
}

std::shared_ptr<jl::value::Variable> jl::Block::create_named_variable(const std::string& name)
{
    auto var = create_varaible(value::Variable::STACK);
    m_symbol_table.insert({ name, var });
    return var;
}

std::optional<std::shared_ptr<jl::value::Variable>> jl::Block::lookup_variable(const std::string& name) const
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

uint32_t jl::Block::create_label()
{
    return m_var_data->label_count++;
}
