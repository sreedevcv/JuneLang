#include "Token.hpp"

jl::Token::Token(TokenType type, std::string& lexeme, int line)
    : m_type(type)
    , m_lexem(lexeme)
    , m_line(line)
{
}

jl::Token::Token(TokenType type, std::string& lexeme, Value value, int line)
: Token(type, lexeme, line)
{
    m_value = value;
}

jl::Token::~Token()
{
}
