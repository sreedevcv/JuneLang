#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "ir/IR.hpp"

namespace jl {
class Function;

struct BasicBlock {
    std::string name;
    size_t idx;
    Function* parent;
    std::vector<ir::Phi*> phis;

    ir::IR* head = nullptr;
    ir::IR* tail = nullptr;

    ir::IR* get_terminator() const;

    std::string get_name() const;

    void remove_ir(ir::IR* ir);

    void replace_ir(ir::IR* ir, ir::IR* new_ir);

    void insert_before(ir::IR* ir, ir::IR* new_ir);

    // Get instructions of a particular type
    template <typename T>
    std::vector<T> get_instrs()
    {
        std::vector<T> result;

        for (auto ptr = head; ptr != nullptr; ptr = ptr->next) {
            if (auto target = dynamic_cast<T>(ptr)) {
                result.push_back(target);
            }
        }

        return result;
    }
};

class BBIterator {
public:
    BBIterator(ir::IR* head)
        : m_curr(head)
    {
    }

    std::optional<ir::IR*> next()
    {
        if (m_curr != nullptr) {
            auto val = m_curr;
            m_curr = m_curr->next;
            return val;
        } else {
            return std::nullopt;
        }
    }

private:
    ir::IR* m_curr = nullptr;
};

}
