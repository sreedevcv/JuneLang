#include "Lexer.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include "ErrorHandler.hpp"

jl::Lexer::Lexer(std::string& file_path)
    : m_file_path(file_path)
{
    std::ifstream file(file_path);

    if (!file.good()) {
        ErrorHandler::error(file_path, 0, "File not found");
        std::exit(1);
    }

    m_file_size = std::filesystem::file_size(file_path);

    std::ostringstream ss;
    ss << file.rdbuf();
    m_source = ss.str();
}

void jl::Lexer::scan()
{
    while (!is_at_end()) {
        start = current;
        scan_token();
    }
}

std::vector<jl::Token> jl::Lexer::get_tokens()
{
    return m_tokens;
}

bool jl::Lexer::is_at_end()
{
    return current >= m_file_size;
}

void jl::Lexer::scan_token()
{
    char c = advance();

    switch (c) {
    case '(':
        add_token(Token::LEFT_PAR);
        break;
    case ')':
        add_token(Token::RIGHT_PAR);
        break;
    case '{':
        add_token(Token::LEFT_BRACE);
        break;
    case '}':
        add_token(Token::RIGHT_BRACE);
        break;
    case ',':
        add_token(Token::COMMA);
        break;
    case '.':
        add_token(Token::DOT);
        break;
    case '-':
        add_token(Token::MINUS);
        break;
    case '+':
        add_token(Token::PLUS);
        break;
    case '*':
        add_token(Token::STAR);
        break;
    case ';':
        add_token(Token::SEMI_COLON);
        break;
    case '!':
        add_token(match('=') ? Token::BANG_EQUAL : Token::BANG);
        break;
    case '=':
        add_token(match('=') ? Token::EQUAL_EQUAL : Token::EQUAL);
        break;
    case '<':
        add_token(match('=') ? Token::LESS_EQUAL : Token::LESS);
        break;
    case '>':
        add_token(match('=') ? Token::GREATER_EQUAL : Token::GREATER);
        break;
    default:
        std::string msg = "No such symbol" + std::to_string(c);
        ErrorHandler::error(m_file_path, line, msg.c_str(), start);
        break;
    }
}

char jl::Lexer::advance()
{
    return m_source[current++];
}

void jl::Lexer::add_token(Token::TokenType type)
{
    std::string lexeme = m_source.substr(start, current - start);
    m_tokens.push_back(Token(type, lexeme, line));
}

bool jl::Lexer::match(char expected)
{
    if (is_at_end())
        return false;
    if (m_source[current] != expected)
        return false;

    current++;
    return true;
}
