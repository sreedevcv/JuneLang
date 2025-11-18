#pragma once

#include "Expr.hpp"
#include "Stmt.hpp"
#include "Token.hpp"
#include "TypeInfo.hpp"

#include <initializer_list>
#include <memory>

namespace jl {
class Parser {
public:
    Parser(std::vector<Token>& tokens, std::string& file_name);
    ~Parser();

    std::unique_ptr<Expr> parse();
    std::vector<std::unique_ptr<Stmt>> parseStatements();

private:
    std::vector<Token> m_tokens;
    std::string m_file_name;
    int m_current = 0;

    std::unique_ptr<Expr> expression();
    std::unique_ptr<Expr> equality();
    std::unique_ptr<Expr> comparison();
    std::unique_ptr<Expr> term();
    std::unique_ptr<Expr> factor();
    std::unique_ptr<Expr> type_cast();
    std::unique_ptr<Expr> unary();
    std::unique_ptr<Expr> primary();
    std::unique_ptr<Expr> assignment();
    std::unique_ptr<Expr> or_expr();
    std::unique_ptr<Expr> and_expr();
    std::unique_ptr<Expr> call();
    std::unique_ptr<Expr> finish_call(std::unique_ptr<Expr> callee);
    std::unique_ptr<Expr> modify_and_assign(Token::TokenType oper_type, std::unique_ptr<Expr> expr);

    std::unique_ptr<Stmt> statement();
    std::unique_ptr<Stmt> declaration();
    std::unique_ptr<Stmt> print_statement();
    std::unique_ptr<Stmt> expr_statement();
    std::unique_ptr<Stmt> var_declaration(bool for_each = false);
    std::unique_ptr<Stmt> if_stmt();
    std::unique_ptr<Stmt> while_statement();
    std::unique_ptr<Stmt> for_statement();
    std::unique_ptr<Stmt> function(const char* kind);
    std::unique_ptr<Stmt> return_statement();
    std::unique_ptr<Stmt> class_declaration();
    std::unique_ptr<Stmt> break_statement();
    std::unique_ptr<Stmt> extern_declaration();
    std::unique_ptr<FuncStmt> function_declaration();
    std::vector<std::unique_ptr<Stmt>> block();

    void synchronize();
    bool match(std::initializer_list<Token::TokenType>&& types);
    bool check(Token::TokenType type);
    bool is_at_end();
    Token& advance();
    Token& peek();
    Token& previous();
    Token& consume(Token::TokenType type, const char* msg);
    std::unique_ptr<Expr> parse_list();
    std::optional<TypeInfo> parse_type_info();
};
} // namespace jl
