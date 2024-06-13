#include "Environment.hpp"

#include "ErrorHandler.hpp"

jl::Environment::Environment()
    : m_enclosing(nullptr)
{
}

jl::Environment::Environment(Environment* enclosing)
    : m_enclosing(enclosing)
{
}

jl::Environment::~Environment()
{
    if (m_enclosing != nullptr) {
        delete m_enclosing;
    }
}

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

    if (m_enclosing != nullptr) {
        return m_enclosing->get(token);
    }

    std::string fname = "unknown";
    ErrorHandler::error(fname, token.get_line(), "variable does not exist");
    throw "exception";
}

void jl::Environment::assign(const Token& token, const Token::Value& value)
{
    if (m_values.contains(token.get_lexeme())) {
        m_values[token.get_lexeme()] = value;
        return;
    }

    if (m_enclosing != nullptr) {
        m_enclosing->assign(token, value);
        return;
    }

    std::string fname = "unknown";
    ErrorHandler::error(fname, token.get_line(), "Udefined variable");
    throw "exception";
}
