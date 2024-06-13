#pragma once

#include <unordered_map>

#include "Token.hpp"

namespace jl {

class Environment {
public:
    /* Stores a copy of variable name and token::value in map if
        they dont already exists otherwise throws an exception */
    void define(const std::string& name, const Token::Value& value);
    /* Retruves the sored reference to a token otherwise
        throws an exception */
    Token::Value& get(const Token& name);

private:
    std::unordered_map<std::string, Token::Value> m_values;
};

} // namespace jl
