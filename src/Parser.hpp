#pragma once

#include "Expr.hpp"
#include "Token.hpp"

namespace jl {
class Parser {
public:
    Parser(std::vector<Token>& tokens);
    ~Parser() = default;

    Expr* parse();
    
private:
    std::vector<Token> m_tokens;

    int m_current = 0;

    Expr* expression();
    Expr* equality();
    Expr* comparison();
    Expr* term();
    Expr* factor();
    Expr* unary();
    Expr* primary();

    bool match(std::vector<Token::TokenType>&& types);
    bool check(Token::TokenType type);
    bool is_at_end();
    Token& advance();
    Token& peek();
    Token& previous();
    Token& consume(Token::TokenType type, const char* msg);
};
} // namespace jl
