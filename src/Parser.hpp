#pragma once

#include "Expr.hpp"
#include "Token.hpp"
#include "Stmt.hpp"

namespace jl {
class Parser {
public:
    Parser(std::vector<Token>& tokens);
    ~Parser() = default;

    Expr* parse();
    std::vector<Stmt*> parseStatements();

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
    Expr* assignment();
    Expr* or_expr();
    Expr* and_expr();

    Stmt* statement();
    Stmt* declaration();
    Stmt* print_statement();
    Stmt* expr_statement();
    Stmt* var_declaration();
    Stmt* if_stmt();
    Stmt* while_statement();
    Stmt* for_statement();
    std::vector<Stmt*> block();

    void synchronize();
    bool match(std::vector<Token::TokenType>&& types);
    bool check(Token::TokenType type);
    bool is_at_end();
    Token& advance();
    Token& peek();
    Token& previous();
    Token& consume(Token::TokenType type, const char* msg);
};
} // namespace jl
