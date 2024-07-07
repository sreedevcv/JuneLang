#pragma once

#include "Expr.hpp"
#include "Token.hpp"
#include "Stmt.hpp"
#include "Arena.hpp"

namespace jl {
class Parser {
public:
    Parser(Arena& arena, std::vector<Token>& tokens, std::string& file_name);
    ~Parser() = default;

    Expr* parse();
    std::vector<Stmt*> parseStatements();

private:
    Arena& m_arena;
    std::vector<Token> m_tokens;
    std::string m_file_name;
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
    Expr* call();
    Expr* finish_call(Expr* callee);

    Stmt* statement();
    Stmt* declaration();
    Stmt* print_statement();
    Stmt* expr_statement();
    Stmt* var_declaration();
    Stmt* if_stmt();
    Stmt* while_statement();
    Stmt* for_statement();
    Stmt* function(const char* kind);
    Stmt* return_statement();
    Stmt* class_declaration();
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
