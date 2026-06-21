#include "LiteralValue.hpp"
#include <format>
#include <variant>

jl::LiteralValue::LiteralValue(const type& data)
    : data(data)
{
}

std::string jl::LiteralValue::to_str() const
{
    return std::visit([](auto&& val) -> std::string {
        return std::format("{}", val);
    },
        data);
}

bool jl::LiteralValue::operator==(const LiteralValue& other) const
{
    return data == other.data;
}