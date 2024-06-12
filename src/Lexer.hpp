#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "Token.hpp"
#include "Expr.hpp"

namespace jl {
class Lexer {
public:
    Lexer(const char* source);
    Lexer(std::string& file_path);
    ~Lexer() = default;
    void scan();
    std::vector<Token> get_tokens();

private:
    std::string m_file_path;
    std::vector<Token> m_tokens;
    std::string m_source;

    int m_line = 1;
    int m_current = 0;
    int m_start = 0;
    // int m_file_size = 0;

    bool match(char expected);
    bool is_at_end();
    bool is_digit(char c);
    bool is_alpha(char c);
    bool is_alphanumeric(char c);

    char advance();
    char peek();
    char peek_next();

    void add_token(Token::TokenType type);
    void add_token(Token::TokenType type, Token::Value value);
    void scan_token();
    void scan_string();
    void scan_number();
    void scan_identifier();

    std::unordered_map<std::string, Token::TokenType> m_reserved_words = {
        { "and", Token::AND },
        { "or", Token::OR },
        { "not", Token::NOT },
        { "if", Token::IF },
        { "then", Token::THEN },
        { "while", Token::WHILE },
        { "do", Token::DO },
        { "for", Token::FOR },
        { "end", Token::END },
        { "true", Token::TRUE },
        { "false", Token::FALSE },
        { "return", Token::RETURN },
        { "var", Token::VAR },
        { "null", Token::NULL_ },
        { "print", Token::PRINT },
    };
};
} // namespace jl
