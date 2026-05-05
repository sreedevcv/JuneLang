#pragma once

#include "backend/ir/IR.hpp"
#include "types/Type.hpp"
#include "value/Variable.hpp"

#include <memory>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

namespace jl {

class FuncBlock {
public:
    FuncBlock(const std::string& name, const type::Func* type);

    ~FuncBlock() = default;

    FuncBlock(const FuncBlock&) = delete;

    FuncBlock& operator=(const FuncBlock&) = delete;

    FuncBlock(FuncBlock&&) = default;

    FuncBlock& operator=(FuncBlock&&) = default;

    template <typename T, typename... Args>
    void add_ir(Args&&... args)
    {
        m_current_block->irs.push_back(std::make_unique<T>(std::forward<Args>(args)...));
        m_current_block->chain(m_current_block->irs.back().get());
    }

    template <typename T>
    void add_ir(T&& ir)
    {
        m_current_block->irs.push_back(std::make_unique<T>(std::forward<T>(ir)));
        m_current_block->chain(m_current_block->irs.back().get());
    }

    struct BasicBlock {
        const type::Func* type;
        std::vector<std::unique_ptr<ir::IR>> irs;
        value::VarData var_data;
        ir::IR* head = nullptr;
        ir::IR* tail = nullptr;

        BasicBlock(const type::Func* func_type)
            : type(func_type)
        {
        }

        void chain(ir::IR* ir)
        {
            if (head == nullptr) {
                head = ir;
                tail = head;
            } else {
                ir->prev = tail;
                tail->next = ir;
                tail = ir;
            }
        }
    };

    void push_func(const std::string& name, const type::Func* type);

    void pop_func();

    uint32_t get_last_line() const;

    std::unordered_map<std::string, std::unique_ptr<BasicBlock>> basic_blocks();

    std::ostream& stream(std::ostream& in) const;

    const std::string& get_current_func_name() const;

    // value::VarData* get_var_data();

private:
    std::unordered_map<std::string, std::unique_ptr<BasicBlock>> m_basic_blocks;
    std::stack<BasicBlock*> m_blks;
    BasicBlock* m_current_block = nullptr;
    std::string m_current_func_name;
};
}
