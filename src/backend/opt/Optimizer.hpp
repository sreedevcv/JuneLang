#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "FuncBlock.hpp"

namespace jl {
class Optimizer {
public:
    Optimizer(std::unordered_map<std::string, std::unique_ptr<FuncBlock::BasicBlock>>& ir_data);

private:
    const std::unordered_map<std::string, std::unique_ptr<FuncBlock::BasicBlock>>& ir_data;
};
}