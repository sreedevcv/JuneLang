#include "Function.hpp"
#include "BasicBlock.hpp"
#include "ir/Phi.hpp"
#include "opt/Optimizer.hpp"
#include "types/Type.hpp"
#include "utils/algorithms.hpp"
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

jl::BasicBlock* jl::Function::entry_block()
{
    if (m_blocks.size() > 0) {
        return m_blocks.front().get();
    } else {
        return nullptr;
    }
}

std::vector<std::unique_ptr<jl::BasicBlock>>& jl::Function::blocks()
{
    return m_blocks;
}

std::vector<std::unique_ptr<jl::ir::IR>>& jl::Function::irs()
{
    return m_irs;
}

void jl::Function::replace_value(jl::value::Variable from, jl::value::Variable to)
{
    for (auto& ir : m_irs) {
        if (ir->uses(from)) {
            ir->replace(from, to);
        }
    }
}

const std::string& jl::Function::name()
{
    return m_name;
}

std::ostream& jl::operator<<(std::ostream& out, Function& function)
{
    // Print the signature
    auto type = static_cast<const type::Func*>(function.m_type);

    out << function.m_name << "::(";

    if (function.m_input_args.size() > 0) {
        out << function.m_input_args.front().to_str();
    }

    for (int i = 1; i < function.m_input_args.size(); i++) {
        out << ", " << function.m_input_args[i].to_str();
    }

    out << ") -> " << type->m_return_type->to_str() << '\n';

    // Print the basic blocks
    for (const auto& block : function.m_blocks) {
        // First print the phi instructions

        for (const auto phi : block->phis) {
            out << std::right << std::setw(5) << '\t' << phi->to_str() << '\n';
        }

        // Then print the other instructions

        out << block->get_name() << ": \n";

        auto ptr = block->head;

        while (ptr != nullptr) {
            out << std::right << std::setw(5) << ptr->m_line << '\t' << ptr->to_str() << '\n';
            ptr = ptr->next;
        }

        out << std::endl;
    }

    // Print rpo
    const auto rpo = algorithms::RPO(function.m_blocks[0].get());

    out << "\nPost Order Traversal:\n";
    for (const auto [block, idx] : rpo) {
        out << "[" << idx << "]: " << block->get_name() << '\n';
    }

    // Print doms
    const auto doms = algorithms::dominance_tree(&function);

    out << "\nDominance Tree:\n";
    for (const auto [block, parent] : doms) {
        out << block->get_name() << " -> " << parent->get_name() << '\n';
    }

    out << "\n";

    return out;
}
