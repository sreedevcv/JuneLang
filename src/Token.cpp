#include "Token.hpp"

#include "Expr.hpp"

jl::Token::Value jl::Token::global_true_constant = jl::Token::Value(true);
jl::Token::Value jl::Token::global_false_constant = jl::Token::Value(false);

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

int jl::Token::get_line() const
{
    return m_line;
}

bool jl::is_int(jl::Token::Value& value)
{
    return value.index() == 0;
}

bool jl::is_float(jl::Token::Value& value)
{
    return value.index() == 1;
}

bool jl::is_bool(jl::Token::Value& value)
{
    return value.index() == 2;
}

bool jl::is_string(jl::Token::Value& value)
{
    return value.index() == 3;
}

bool jl::is_null(jl::Token::Value& value)
{
    return value.index() == 4;
}

bool jl::is_number(Token::Value& value)
{
    return is_int(value) || is_float(value);
}
