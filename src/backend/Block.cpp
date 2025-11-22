#include "Block.hpp"
#include "backend/value/Variable.hpp"
#include <memory>

jl::Block::Block(const Block* parent, uint32_t id)
    : m_parent(parent)
    , m_id(id)
{
}

jl::Block* jl::Block::create_block()
{
    auto block = std::make_unique<Block>(this, m_children.size());
    auto ref = block.get();
    m_children.push_back(std::move(block));
    return ref;
}

std::shared_ptr<jl::value::Variable> jl::Block::create_varaible()
{
    return std::make_unique<value::Variable>(m_var_count++);
}

std::shared_ptr<jl::value::Variable> jl::Block::create_named_variable(const std::string& name)
{
    auto var = create_varaible();
    m_symbol_table.insert({ name, var });
    return var;
}

std::optional<std::shared_ptr<jl::value::Variable>> jl::Block::lookup_variable(const std::string& name)
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

uint32_t jl::Block::get_last_line() const
{
    if (m_irs.size() > 0) {
        return m_irs.back().get()->m_line;
    } else {
        return 0;
    }
}
