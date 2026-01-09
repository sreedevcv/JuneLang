#pragma once

#include "backend/ir/IR.hpp"
#include "backend/types/Type.hpp"
#include "value/Variable.hpp"

#include <memory>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

namespace jl {

class FuncBlock {
public:
    FuncBlock(const std::string& name, std::unique_ptr<type::Func> type);

    ~FuncBlock() = default;

    FuncBlock(const FuncBlock&) = delete;

    FuncBlock& operator=(const FuncBlock&) = delete;

    FuncBlock(FuncBlock&&) = default;

    FuncBlock& operator=(FuncBlock&&) = default;

    template <typename T, typename... Args>
    void add_ir(Args&&... args)
    {
        m_current_func->irs.push_back(std::make_unique<T>(std::forward<Args>(args)...));
    }

    struct FuncData {
        std::unique_ptr<type::Func> type;
        std::vector<std::unique_ptr<ir::IR>> irs;
        value::VarData var_data;

        FuncData(std::unique_ptr<type::Func> func_type)
            : type(std::move(func_type))
        {
        }
    };

    void push_func(const std::string& name, std::unique_ptr<type::Func> type);

    void pop_func();

    uint32_t get_last_line() const;

    std::unordered_map<std::string, std::unique_ptr<FuncData>> get_func_irs();

    std::ostream& stream(std::ostream& in) const;

    const std::string& get_current_func_name() const;

    value::VarData* get_var_data();

private:
    std::unordered_map<std::string, std::unique_ptr<FuncData>> m_func_datas;
    std::stack<FuncData*> m_funcs;
    FuncData* m_current_func = nullptr;
    std::string m_current_func_name;
};
}
