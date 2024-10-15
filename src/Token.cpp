#include "Token.hpp"

jl::JlValue jl::Token::global_true_constant = jl::JlValue(true);
jl::JlValue jl::Token::global_false_constant = jl::JlValue(false);
std::string jl::Token::global_super_lexeme = "super";
std::string jl::Token::global_this_lexeme = "self";
// jl::Token jl::Token::global_plus_equal = Token(TokenType::PLUS_EQUAL,);

jl::Token::Token(TokenType type, std::string& lexeme, int line)
    : m_type(type)
    , m_lexeme(lexeme)
    , m_line(line)
{
}

jl::Token::Token(TokenType type, std::string& lexeme, int line, JlValue value)
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

jl::JlValue jl::Token::get_value() const
{
    return m_value;
}

int jl::Token::get_line() const
{
    return m_line;
}

bool jl::is_int(jl::JlValue* value)
{
    return value->index() == 0;
}

bool jl::is_float(jl::JlValue* value)
{
    return value->index() == 1;
}

bool jl::is_bool(jl::JlValue* value)
{
    return value->index() == 2;
}

bool jl::is_string(jl::JlValue* value)
{
    return value->index() == 3;
}

bool jl::is_null(jl::JlValue* value)
{
    return value->index() == 4;
}

bool jl::is_number(JlValue* value)
{
    return is_int(value) || is_float(value);
}

bool jl::is_callable(JlValue* value)
{
    return value->index() == 5;
}

bool jl::is_instance(JlValue* value)
{
    return value->index() == 6;
}

bool jl::is_jlist(JlValue* value)
{
    return value->index() == 7;
}
