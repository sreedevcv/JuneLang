#include "Token.hpp"

jl::Value jl::Token::global_true_constant = jl::Value(true);
jl::Value jl::Token::global_false_constant = jl::Value(false);
std::string jl::Token::global_super_lexeme = "super";
std::string jl::Token::global_this_lexeme = "self";
// jl::Token jl::Token::global_plus_equal = Token(TokenType::PLUS_EQUAL,);

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

std::string& jl::Token::get_lexeme()
{
    return m_lexeme;
}

jl::Value jl::Token::get_value() const
{
    return m_value;
}

int jl::Token::get_line() const
{
    return m_line;
}

bool jl::is_int(jl::Value& value)
{
    return value.index() == 0;
}

bool jl::is_float(jl::Value& value)
{
    return value.index() == 1;
}

bool jl::is_bool(jl::Value& value)
{
    return value.index() == 2;
}

bool jl::is_string(jl::Value& value)
{
    return value.index() == 3;
}

bool jl::is_null(jl::Value& value)
{
    return value.index() == 4;
}

bool jl::is_number(Value& value)
{
    return is_int(value) || is_float(value);
}

bool jl::is_callable(Value& value)
{
    return value.index() == 5;
}

bool jl::is_instance(Value& value)
{
    return value.index() == 6;
}

bool jl::is_jlist(Value& value)
{
    return value.index() == 7;
}
