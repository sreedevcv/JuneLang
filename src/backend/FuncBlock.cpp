#include "FuncBlock.hpp"
#include "backend/ir/IR.hpp"
#include "backend/ir/Jump.hpp"

#include <memory>
#include <utility>
#include <vector>

jl::FuncBlock::FuncBlock(const std::string& name, const type::Func* type)
{
    push_func(name, type);
}

uint32_t jl::FuncBlock::get_last_line() const
{
    if (m_current_block->irs.size() > 0) {
        return m_current_block->irs.back().get()->line();
    } else {
        return 0;
    }
}

std::ostream& jl::FuncBlock::stream(std::ostream& in) const
{
    for (const auto& [name, func_info] : m_basic_blocks) {
        in << name << " : " << func_info->type->to_str() << "\n";

        for (const auto& ir : func_info->irs) {
            in << ir->line() << '\t';

            if (!dynamic_cast<ir::Label*>(ir.get())) {
                in << '\t';
            }

            in << ir->to_str() << '\n';
        }

        in << '\n';
    }

    return in;
}

void jl::FuncBlock::push_func(const std::string& name, const type::Func* type)
{
    auto func_data = std::make_unique<BasicBlock>(type);
    m_blks.push(func_data.get());
    m_basic_blocks.insert({ name, std::move(func_data) });
    m_current_func_name = name;
    m_current_block = m_blks.top();
}

void jl::FuncBlock::pop_func()
{
    m_blks.pop();
    m_current_block = m_blks.top();
}

std::unordered_map<std::string, std::unique_ptr<jl::FuncBlock::BasicBlock>> jl::FuncBlock::basic_blocks()
{
    return std::move(m_basic_blocks);
}

const std::string& jl::FuncBlock::get_current_func_name() const
{
    return m_current_func_name;
}

jl::value::VarData* jl::FuncBlock::get_var_data()
{
    return &m_blks.top()->var_data;
}
