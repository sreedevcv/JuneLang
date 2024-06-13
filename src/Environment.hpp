#pragma once

#include <unordered_map>

#include "Token.hpp"

namespace jl {

class Environment {
public:
    Environment();
    Environment(Environment *enclosing);

    /* Stores a copy of variable name and token::value in map if
        they dont already exists otherwise throws an exception */
    void define(const std::string& name, const Token::Value& value);
    /* Retrives the sored reference to a token otherwise
        throws an exception */
    Token::Value& get(const Token& name);

    void assign(const Token& token, const Token::Value& value);

private:
    std::unordered_map<std::string, Token::Value> m_values;
    Environment* m_enclosing;
};

} // namespace jl
