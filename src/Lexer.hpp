#pragma once

#include <string>
#include <vector>

#include "Token.hpp"

#include "Token.hpp"

namespace jl {
class Lexer {
public:
    Lexer(std::string& file_path);
    ~Lexer() = default;
    void scan();

private:
    std::string m_file_path;
    std::vector<Token> m_tokens;
    std::string m_source;

    int line = 1;
    int current = 0;
    int start = 0;
    size_t m_file_size = 0;

    bool is_at_end();
    void scan_token();
    char advance();
    void add_token(Token::TokenType type);
    bool match(char expected);
};
} // namespace jl
