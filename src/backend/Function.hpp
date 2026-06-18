#pragma once

#include <cstdint>
#include <list>
#include <memory>
#include <string>
#include <vector>

#include "BasicBlock.hpp"
#include "ir/AllocateVar.hpp"
#include "ir/IR.hpp"
#include "types/Type.hpp"
#include "value/Variable.hpp"

namespace jl {
class Module;

class Function {
public:
    Function(std::string name, const type::Type* type);

    BasicBlock* new_block(std::string_view name);

    BasicBlock* current_block();

    void set_current_block(BasicBlock* block);

    template <typename T, typename... Args>
    void add_ir(Args&&... args)
    {
        m_irs.push_back(std::make_unique<T>(std::forward<Args>(args)...));
        chain(m_irs.back().get());
    }

    template <typename T>
    void add_ir(T&& ir)
    {
        m_irs.push_back(std::make_unique<T>(std::forward<T>(ir)));
        chain(m_irs.back().get());
    }

    // Specialization for AllocateVar instrs so that they are placed in the entry block
    void add_ir(ir::AllocateVar alloca)
    {
        m_irs.push_back(std::make_unique<ir::AllocateVar>(alloca));
        auto new_instr = m_irs.back().get();

        entry_block()->tail->next = new_instr;
        new_instr->next = nullptr;
        new_instr->prev = entry_block()->tail;
        entry_block()->tail = new_instr;
    }

    uint32_t m_var_count = 0;

    void add_input_arg(value::Variable var);

    BasicBlock* entry_block();

    std::vector<std::unique_ptr<BasicBlock>>& blocks();

    std::list<std::unique_ptr<ir::IR>>& irs();

    const std::string& name();

    void replace_value(jl::value::Variable from, jl::value::Variable to);

    void remove_ir(BasicBlock* block, ir::IR* ir);

private:
    std::string m_name;
    const type::Type* m_type;
    std::vector<std::unique_ptr<BasicBlock>> m_blocks;
    std::list<std::unique_ptr<ir::IR>> m_irs;
    BasicBlock* m_current_block = nullptr;
    std::vector<value::Variable> m_input_args;

    void chain(ir::IR* ir)
    {
        if (m_current_block->head == nullptr) {
            m_current_block->head = ir;
            m_current_block->tail = m_current_block->head;
        } else {
            auto newest = ir;
            m_current_block->tail->next = newest;
            newest->prev = m_current_block->tail;
            m_current_block->tail = newest;
        }
    }

    friend std::ostream& operator<<(std::ostream& out, Function& function);
};

std::ostream& operator<<(std::ostream& out, Function& function);
}
