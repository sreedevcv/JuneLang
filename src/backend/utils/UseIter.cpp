#include "UseIter.hpp"
#include "BasicBlock.hpp"

jl::util::UseIter::UseIter(jl::BasicBlock* block, value::Variable def)
    : m_block(block)
    , m_def(def)
    , m_ptr(block->head)
{
}

bool jl::util::UseIter::has_next()
{
    if (m_ptr == nullptr) {
        return false;
    } else {
        while (m_ptr != nullptr) {
            if (m_ptr->uses(m_def)) {
                m_ptr = m_ptr->next;
                return true;
            }

            m_ptr = m_ptr->next;
        }

        return false;
    }
}

jl::ir::IR* jl::util::UseIter::next()
{
    return m_ptr;
}
