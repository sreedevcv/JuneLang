#include "Module.hpp"

jl::Function* jl::Module::create_function(std::string_view name, const type::Type* type)
{
    m_functions.emplace_back(
        std::make_unique<Function>(
            std::string { name.begin(), name.end() },
            type));

    set_current_function(m_functions.back().get());
    return current_function();
}

jl::Function* jl::Module::current_function()
{
    return m_current_func;
}

void jl::Module::set_current_function(Function* function)
{
    m_current_func = function;
}

std::ostream& jl::operator<<(std::ostream& out, const jl::Module& module)
{
    for (const auto& function : module.m_functions) {
        out << *function.get();
    }
    return out;
}

jl::Function* jl::Module::get_function(std::string_view name)
{
    for (auto& func : m_functions) {
        if (func->name() == name) {
            return func.get();
        }
    }

    return nullptr;
}

const std::vector<std::unique_ptr<jl::Function>>& jl::Module::functions() const
{
    return m_functions;
}