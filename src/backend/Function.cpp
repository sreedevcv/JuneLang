#include "Function.hpp"
#include "BasicBlock.hpp"
#include "types/Type.hpp"
#include <iomanip>
#include <ios>
#include <ostream>

jl::Function::Function(std::string name, const type::Type* type)
    : m_name(name)
    , m_type(type)
{
}

jl::BasicBlock* jl::Function::new_block(std::string_view name)
{
    m_blocks.push_back(std::make_unique<BasicBlock>(
        BasicBlock {
            .name = { name.begin(), name.end() },
            .idx = m_blocks.size(),
            .parent = this,
        }));

    return m_blocks.back().get();
}

jl::BasicBlock* jl::Function::current_block()
{
    return m_current_block;
}

void jl::Function::set_current_block(BasicBlock* block)
{
    m_current_block = block;
}

void jl::Function::add_input_arg(value::Variable var)
{
    m_input_args.push_back(var);
}

std::ostream& jl::operator<<(std::ostream& out, const Function& function)
{
    auto type = static_cast<const type::Func*>(function.m_type);

    out << function.m_name << "::(";

    if (function.m_input_args.size() > 0) {
        out << function.m_input_args.front().to_str();
    }

    for (int i = 1; i < function.m_input_args.size(); i++) {
        out << ", " << function.m_input_args[i].to_str();
    }

    out << ") -> " << type->m_return_type->to_str() << '\n';

    for (const auto& block : function.m_blocks) {
        out << block->get_name() << ": \n";

        auto ptr = block->head;

        while (ptr != nullptr) {
            out << std::right << std::setw(5) << ptr->m_line << '\t' << ptr->to_str() << '\n';
            ptr = ptr->next;
        }

        out << std::endl;
    }

    return out;
}