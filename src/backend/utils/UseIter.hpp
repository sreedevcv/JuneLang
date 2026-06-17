#pragma once

#include "BasicBlock.hpp"
#include "value/Variable.hpp"

namespace jl {
namespace util {

    class UseIter {
    public:
        UseIter(BasicBlock* block, value::Variable def);

        bool has_next();

        ir::IR* next();

    private:
        BasicBlock* m_block;
        value::Variable m_def;
        ir::IR* m_ptr = nullptr;
    };

}
}
