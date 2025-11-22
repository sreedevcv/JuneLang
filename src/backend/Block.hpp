#pragma once

#include "backend/ir/IR.hpp"
#include "backend/value/Variable.hpp"
#include <cstdint>
#include <memory>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>

namespace jl {
class Block {
public:
    Block(const Block* parent, uint32_t id);

    template <typename T, typename... Args>
    void add_ir(Args&&... args)
    {
        m_irs.push_back(std::make_unique<T>(std::forward<Args>(args)...));
    }

    Block* create_block();
    std::shared_ptr<value::Variable> create_varaible();
    std::shared_ptr<value::Variable> create_named_variable(const std::string& name);
    std::optional<std::shared_ptr<value::Variable>> lookup_variable(const std::string& name);

    uint32_t get_last_line() const;

protected:
    std::vector<std::unique_ptr<ir::IR>> m_irs;
    std::unordered_map<std::string, std::shared_ptr<value::Variable>> m_symbol_table;
    std::vector<std::unique_ptr<Block>> m_children;

    uint32_t m_var_count { 0 };
    const Block* m_parent;
    uint32_t m_id;
};

}
