#include "Optimizer.hpp"

#include "BasicBlock.hpp"
#include "Function.hpp"
#include "ir/AllocateVar.hpp"
#include "ir/ConditionalJump.hpp"
#include "ir/IR.hpp"
#include "ir/Jump.hpp"
#include "ir/Phi.hpp"
#include "ir/Read.hpp"
#include "ir/Write.hpp"
#include "value/Variable.hpp"
#include <optional>

jl::ir::IR* remove_phi_node(jl::Function* function, jl::ir::Phi* phi, jl::ir::IR* insertion_point)
{
    function->set_current_block(function->entry_block());
    auto addr = jl::value::Variable(function->m_var_count++, phi->m_dest.type());
    auto alloca = function->add_ir_to_front(jl::ir::AllocateVar(addr, phi->m_dest.type(), phi->m_line));

    for (auto [var, block] : phi->m_opers) {
        auto write = jl::ir::Write(
            var,
            addr,
            std::nullopt,
            var.type()->size(),
            phi->line());

        function->add_ir_before(std::move(write), block->get_terminator());
    }
    auto read = jl::ir::Read(
        phi->m_dest,
        addr,
        std::nullopt,
        phi->m_dest.type()->size(),
        phi->m_line);

    function->set_current_block(phi->parent);

    // Insertion point is used to add the reads of phi in the order they were created
    // If insertion_point is null, then the new read will be the first instr of the block,
    // otherwise we will add it after the insertion point(which would be the previously inserted read)
    jl::ir::IR* new_insertion_point = nullptr;

    if (insertion_point == nullptr) {
        new_insertion_point = function->add_ir_to_front(std::move(read));
    } else {
        new_insertion_point = function->add_ir_after(std::move(read), insertion_point);
    }

    return new_insertion_point;
}

void jl::opt::remove_phi_nodes(jl::Function* function)
{
    for (auto& block : function->blocks()) {
        jl::ir::IR* insertion_point = nullptr;

        for (auto phi : block->phis) {
            insertion_point = remove_phi_node(function, phi, insertion_point);
        }

        block->phis.clear();
    }
}
