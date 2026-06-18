#pragma once

#include <memory>
#include <ostream>
#include <string_view>
#include <vector>

#include "Function.hpp"
#include "types/Type.hpp"

namespace jl {
class Module {
public:
    Function* create_function(std::string_view name, const type::Type* type);

    Function* current_function();

    Function* get_function(std::string_view name);

    void set_current_function(Function* function);

    const std::vector<std::unique_ptr<Function>>& functions() const;

private:
    std::vector<std::unique_ptr<Function>> m_functions;
    Function* m_current_func;

    friend std::ostream& operator<<(std::ostream& out, const Module& module);
};

std::ostream& operator<<(std::ostream& out, const Module& module);
}