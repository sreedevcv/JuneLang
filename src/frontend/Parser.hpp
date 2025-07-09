#pragma once

#include "Expr.hpp"
#include "Stmt.hpp"
#include "Token.hpp"
#include <initializer_list>

namespace jl {
class Parser {
public:
    Parser(std::vector<Token>& tokens, std::string& file_name);
    ~Parser();

    Expr* parse();
    std::vector<Stmt*> parseStatements();

private:
    std::vector<Token> m_tokens;
    std::vector<Ref*> m_allocated_refs;
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
    Expr* modify_and_assign(Token::TokenType oper_type, Expr* expr);

    Stmt* statement();
    Stmt* declaration();
    Stmt* print_statement();
    Stmt* expr_statement();
    Stmt* var_declaration(bool for_each = false);
    Stmt* if_stmt();
    Stmt* while_statement();
    Stmt* for_statement();
    Stmt* function(const char* kind);
    Stmt* return_statement();
    Stmt* class_declaration();
    Stmt* break_statement();
    Stmt* extern_declaration();
    FuncStmt* function_declaration();
    std::vector<Stmt*> block();

    void synchronize();
    bool match(std::initializer_list<Token::TokenType>&& types);
    bool check(Token::TokenType type);
    bool is_at_end();
    Token& advance();
    Token& peek();
    Token& previous();
    Token& consume(Token::TokenType type, const char* msg);
    Expr* parse_list();
};
} // namespace jl
