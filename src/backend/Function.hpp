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

    template <typename T>
    T* add_ir_to_front(T&& ir)
    {
        m_irs.push_back(std::make_unique<T>(std::forward<T>(ir)));

        auto new_ir = m_irs.back().get();
        new_ir->parent = m_current_block;

        if (m_current_block->head == nullptr) {
            m_current_block->head = new_ir;
            m_current_block->tail = new_ir;
        } else {
            m_current_block->head->prev = new_ir;
            new_ir->next = m_current_block->head;
            m_current_block->head = new_ir;
        }

        return static_cast<T*>(m_irs.back().get());
    }

    template <typename T>
    T* add_ir_before(T&& ir, ir::IR* point)
    {
        m_irs.push_back(std::make_unique<T>(std::forward<T>(ir)));

        auto new_ir = m_irs.back().get();
        new_ir->parent = m_current_block;

        if (m_current_block->head == point) {
            m_current_block->head = new_ir;
            new_ir->next = point;
            point->prev = new_ir;
        } else {
            new_ir->prev = point->prev;
            new_ir->next = point;
            point->prev->next = new_ir;
            point->prev = new_ir;
        }

        return static_cast<T*>(m_irs.back().get());
    }
    template <typename T>
    T* add_ir_after(T&& ir, ir::IR* point)
    {
        m_irs.push_back(std::make_unique<T>(std::forward<T>(ir)));

        auto new_ir = m_irs.back().get();
        new_ir->parent = m_current_block;

        if (m_current_block->tail == point) {
            m_current_block->tail = new_ir;
            new_ir->prev = point;
            point->next = new_ir;
        } else {
            new_ir->next = point->next;
            new_ir->prev = point;
            point->next->prev = new_ir;
            point->next = new_ir;
        }

        return static_cast<T*>(m_irs.back().get());
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

    std::list<std::unique_ptr<BasicBlock>>& blocks();

    std::list<std::unique_ptr<ir::IR>>& irs();

    const std::vector<value::Variable>& args() const;

    const std::string& name() const;

    const type::Func* type() const;

    void replace_value(jl::value::Variable from, jl::value::Variable to);

    void remove_ir(ir::IR* ir);

    void replace_ir(ir::IR* ir, ir::IR* new_ir);

    void remove_block(BasicBlock* block);

private:
    std::string m_name;
    const type::Func* m_type;
    std::list<std::unique_ptr<BasicBlock>> m_blocks;
    std::list<std::unique_ptr<ir::IR>> m_irs;
    BasicBlock* m_current_block = nullptr;
    std::vector<value::Variable> m_input_args;

    void chain(ir::IR* ir)
    {
        ir->parent = m_current_block;

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
