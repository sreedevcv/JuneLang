#pragma once

#include "backend/Block.hpp"
#include "backend/value/Variable.hpp"

#include <string>
#include <unordered_map>

namespace jl {

class FuncBlock : public Block {
public:
    FuncBlock(const Block* parent, uint32_t id, const std::string& name);

    std::shared_ptr<value::Variable> add_input_parameter(const std::string& name);

private:
    std::string m_name;
    std::unordered_map<std::string, value::Variable*> m_inputs;
};
}
