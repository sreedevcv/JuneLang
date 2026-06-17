#pragma once

#include "BasicBlock.hpp"
#include "ir/IR.hpp"
#include "value/Variable.hpp"
#include <vector>

namespace jl {
namespace ir {

    struct Phi : public IR {
        Phi(value::Variable dest, value::Variable replacing_addr);

        std::string to_str() const override;

        void accept(IRVisitor& visitor) override;

        bool uses(value::Variable variable) override;

        void replace(value::Variable from, value::Variable to) override;

        value::Variable m_replacing_addr;
        value::Variable m_dest;
        std::vector<std::pair<value::Variable, BasicBlock*>> m_opers;
    };

}
}
