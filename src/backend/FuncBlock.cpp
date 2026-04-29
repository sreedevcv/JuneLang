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
    if (m_current_func->irs.size() > 0) {
        return m_current_func->irs.back().get()->line();
    } else {
        return 0;
    }
}

std::ostream& jl::FuncBlock::stream(std::ostream& in) const
{
    for (const auto& [name, func_info] : m_func_datas) {
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
    auto func_data = std::make_unique<FuncData>(type);
    m_funcs.push(func_data.get());
    m_func_datas.insert({ name, std::move(func_data) });
    m_current_func_name = name;
    m_current_func = m_funcs.top();
}

void jl::FuncBlock::pop_func()
{
    m_funcs.pop();
    m_current_func = m_funcs.top();
}

std::unordered_map<std::string, std::unique_ptr<jl::FuncBlock::FuncData>> jl::FuncBlock::get_func_irs()
{
    return std::move(m_func_datas);
}

const std::string& jl::FuncBlock::get_current_func_name() const
{
    return m_current_func_name;
}

jl::value::VarData* jl::FuncBlock::get_var_data()
{
    return &m_funcs.top()->var_data;
}
