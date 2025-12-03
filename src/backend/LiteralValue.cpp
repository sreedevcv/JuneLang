#include "LiteralValue.hpp"
#include <format>
#include <variant>

jl::LiteralValue::LiteralValue(const type& data)
    : m_data(data)
{
}


std::string jl::LiteralValue::to_str() const
{
    return std::visit([](auto&& val) -> std::string {
        return std::format("{}", val);
    }, m_data);
}