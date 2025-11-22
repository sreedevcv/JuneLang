#include "FuncBlock.hpp"

#include "backend/Block.hpp"

jl::FuncBlock::FuncBlock(const Block* parent, uint32_t id, const std::string& name)
    : Block(parent, id)
    , m_name(name)
{
}

std::shared_ptr<jl::value::Variable> jl::FuncBlock::add_input_parameter(const std::string& name)
{
    auto var = create_named_variable(name);
    m_inputs.insert({ name, var.get() });
    return var;
}
