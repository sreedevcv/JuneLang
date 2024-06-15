#include "Environment.hpp"

#include "ErrorHandler.hpp"

jl::Environment::Environment(std::string& file_name)
    : m_enclosing(nullptr)
    , m_file_name(file_name)
{
}

jl::Environment::Environment(Environment* enclosing)
    : m_enclosing(enclosing)
    , m_file_name(enclosing->m_file_name)
{
}

jl::Environment::~Environment()
{
    if (m_enclosing != nullptr) {
        delete m_enclosing;
    }
}

void jl::Environment::define(const std::string& name, const Value& value)
{
    if (!m_values.contains(name)) {
        m_values[name] = value;
    } else {
        ErrorHandler::error(m_file_name, 0, "variable already exists");
        throw "exception";
    }
}

jl::Value& jl::Environment::get_ref(const Token& token)
{
    if (m_values.contains(token.get_lexeme())) {
        return m_values[token.get_lexeme()];
    }

    if (m_enclosing != nullptr) {
        return m_enclosing->get_ref(token);
    }

    ErrorHandler::error(m_file_name, token.get_line(), "variable does not exist");
    throw "exception";
}

jl::Value jl::Environment::get_copy(const Token& token)
{
    if (m_values.contains(token.get_lexeme())) {
        return m_values[token.get_lexeme()];
    }

    if (m_enclosing != nullptr) {
        return m_enclosing->get_copy(token);
    }

    ErrorHandler::error(m_file_name, token.get_line(), "variable does not exist");
    throw "exception";
}

void jl::Environment::assign(const Token& token, const Value& value)
{
    if (m_values.contains(token.get_lexeme())) {
        m_values[token.get_lexeme()] = value;
        return;
    }

    if (m_enclosing != nullptr) {
        m_enclosing->assign(token, value);
        return;
    }

    ErrorHandler::error(m_file_name, token.get_line(), "Udefined variable");
    throw "exception";
}
