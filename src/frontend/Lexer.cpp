#include "Lexer.hpp"

#include <fstream>
#include <sstream>
#include <string>

#include "ErrorHandler.hpp"
#include "Token.hpp"
#include "Value.hpp"

jl::Lexer::Lexer(const char* source)
{
    m_source = source;
    m_file_path = "REPL";
}

jl::Lexer::Lexer(std::string& file_path)
    : m_file_path(file_path)
{
    std::ifstream file(file_path);

    if (!file.good()) {
        ErrorHandler::error(file_path, 0, "File not found");
        std::exit(1);
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    m_source = ss.str();
}

jl::Lexer::~Lexer()
{
    for (auto ref : m_allocated_refs) {
        delete ref;
    }
}

void jl::Lexer::scan()
{
    while (!is_at_end()) {
        m_start = m_current;
        scan_token();
    }

    add_token(Token::END_OF_FILE);
}

std::vector<jl::Token> jl::Lexer::get_tokens()
{
    return m_tokens;
}

bool jl::Lexer::is_at_end()
{
    return m_current >= m_source.size();
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
    case '[':
        add_token(Token::LEFT_SQUARE);
        break;
    case ']':
        add_token(Token::RIGHT_SQUARE);
        break;
    case ',':
        add_token(Token::COMMA);
        break;
    case ':':
        add_token(Token::COLON);
        break;
    case '.':
        add_token(Token::DOT);
        break;
    case '&':
        add_token(Token::BIT_AND);
        break;
    case '|':
        add_token(Token::BIT_OR);
        break;
    case '^':
        add_token(Token::BIT_XOR);
        break;
    case '~':
        add_token(Token::BIT_NOT);
        break;
    case '\n':
        m_line++;
        break;
    case ';':
        add_token(Token::SEMI_COLON);
        break;
    case '!':
        add_token(match('=') ? Token::BANG_EQUAL : Token::BANG);
        break;
    case '+':
        add_token(match('=') ? Token::PLUS_EQUAL : Token::PLUS);
        break;
    case '-':
        add_token(match('=') ? Token::MINUS_EQUAL : Token::MINUS);
        break;
    case '*':
        add_token(match('=') ? Token::STAR_EQUAL : Token::STAR);
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
    case '%':
        add_token(match('=') ? Token::PERCENT_EQUAL : Token::PERCENT);
        break;
    case ' ':
    case '\t':
    case '\r':
        break;

    case '/':
        if (match('/')) {
            while (peek() != '\n' && !is_at_end()) {
                advance();
            }
        } else if (match('=')) {
            add_token(Token::SLASH_EQUAL);
        } else {
            add_token(Token::SLASH);
        }
        break;
    case '\'': {
        char ch = advance();
        if (advance() != '\'') {
            ErrorHandler::error(m_file_path, m_line, "Expected a  closing ' for a charachter", m_start);
        } else {
            add_token(Token::CHAR, new Value { ch });
        }
    } break;
    case '"':
        scan_string();
        break;
    default:
        if (is_digit(c)) {
            scan_number();
        } else if (is_alpha(c)) {
            scan_identifier();
        } else {
            std::string msg = "No such symbol" + std::to_string(c);
            ErrorHandler::error(m_file_path, m_line, msg.c_str(), m_start);
        }
        break;
    }
}

char jl::Lexer::advance()
{
    return m_source[m_current++];
}

void jl::Lexer::add_token(Token::TokenType type)
{
    std::string lexeme = m_source.substr(m_start, m_current - m_start);
    m_tokens.push_back(Token(type, lexeme, m_line));
}

void jl::Lexer::add_token(Token::TokenType type, Value* value)
{
    std::string lexeme = m_source.substr(m_start, m_current - m_start);
    m_tokens.push_back(Token(type, lexeme, m_line, value));
}

bool jl::Lexer::match(char expected)
{
    if (is_at_end())
        return false;
    if (m_source[m_current] != expected)
        return false;

    m_current++;
    return true;
}

char jl::Lexer::peek()
{
    if (is_at_end()) {
        return '\0';
    }

    return m_source[m_current];
}

void jl::Lexer::scan_string()
{
    while (peek() != '"' && !is_at_end()) {
        if (peek() == '\n') {
            m_line++;
        }
        advance();
    }

    if (is_at_end()) {
        ErrorHandler::error(m_file_path, m_line, "Unterminated string, expected \"", m_current);
        return;
    }

    advance(); // Read the ending '"'
    std::string value = m_source.substr(m_start + 1, (m_current - 1) - (m_start + 1));
    Value* str = new Value { value }; // FIXME::memory leak!
    m_allocated_refs.push_back(str);
    add_token(Token::STRING, str);
}

bool jl::Lexer::is_digit(char c)
{
    return c >= '0' && c <= '9';
}

void jl::Lexer::scan_number()
{
    bool is_float = false;

    while (is_digit(peek()) && !is_at_end()) {
        advance();
    }

    if (peek() == '.') {
        if (is_digit(peek_next())) {
            advance();
            is_float = true;
            while (is_digit(peek())) {
                advance();
            }
        } else {
            ErrorHandler::error(m_file_path, m_line, "Expected digits after '.'");
            return;
        }
    }

    Value* val;
    if (is_float) {
        val = new Value { std::stod(m_source.substr(m_start, m_current - m_start)) };
        add_token(Token::FLOAT, val);
    } else {
        val = new Value { std::stoi(m_source.substr(m_start, m_current - m_start)) };
        add_token(Token::INT, val);
    }
    m_allocated_refs.push_back(val);
}

char jl::Lexer::peek_next()
{
    if (m_current + 1 >= m_source.size()) {
        return '\0';
    }

    return m_source[m_current + 1];
}

bool jl::Lexer::is_alpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool jl::Lexer::is_alphanumeric(char c)
{
    return is_digit(c) || is_alpha(c);
}

void jl::Lexer::scan_identifier()
{
    while (is_alphanumeric(peek())) {
        advance();
    }

    std::string lexeme = m_source.substr(m_start, m_current - m_start);
    Value* val;

    // FIXME:: Leaks!!!
    if (m_reserved_words.contains(lexeme)) {
        if (lexeme == "true") {
            val = new Value { true };
            add_token(Token::TRUE, val);
            m_allocated_refs.push_back(val);
        } else if (lexeme == "false") {
            val = new Value { false };
            add_token(Token::FALSE, val);
            m_allocated_refs.push_back(val);
        } else if (lexeme == "null") {
            val = new Value { Null {} };
            add_token(Token::NULL_, val);
            m_allocated_refs.push_back(val);
        } else {
            add_token(m_reserved_words[lexeme]);
        }
    } else {
        add_token(Token::IDENTIFIER);
    }
}
