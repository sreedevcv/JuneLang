#include "Token.hpp"

jl::Token::Token(TokenType type, std::string& lexeme, int line)
    : m_type(type)
    , m_lexeme(lexeme)
    , m_line(line)
{
}

jl::Token::Token(TokenType type, std::string& lexeme, int line, Value value)
: Token(type, lexeme, line)
{
    m_value = value;
}

jl::Token::~Token()
{
}

jl::Token::TokenType jl::Token::get_tokentype() const
{
    return m_type;
}

const std::string& jl::Token::get_lexeme() const
{
    return m_lexeme;
}

const jl::Token::Value jl::Token::get_value() const
{
    return m_value;
}

int jl::Token::get_int() const
{
    return std::get<0>(m_value);
}

double jl::Token::get_float() const
{
    return std::get<1>(m_value);
}
