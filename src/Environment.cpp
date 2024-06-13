#include "Environment.hpp"

#include "ErrorHandler.hpp"

void jl::Environment::define(const std::string& name, const Token::Value& value)
{
    if (!m_values.contains(name)) {
        m_values[name] = value;
    } else {
        std::string fname = "unknown";
        ErrorHandler::error(fname, 0, "variable already exists");
        throw "exception";
    }
}

jl::Token::Value& jl::Environment::get(const Token& token)
{
    if (m_values.contains(token.get_lexeme())) {
        return m_values[token.get_lexeme()];
    }

    std::string fname = "unknown";
    ErrorHandler::error(fname, token.get_line(), "variable does not exist");
    throw "exception";
}
