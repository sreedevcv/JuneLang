#pragma once

#include <cstddef>
#include <string>

#include "ir/IR.hpp"

namespace jl {
class Function;

struct BasicBlock {
    std::string name;
    size_t idx;
    Function* parent;

    ir::IR* head = nullptr;
    ir::IR* tail = nullptr;

    ir::IR* get_terminator() const;
    std::string get_name() const;
};

}