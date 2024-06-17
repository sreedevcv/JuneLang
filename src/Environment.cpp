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

jl::Value& jl::Environment::get(Token& token)
{
    if (m_values.contains(token.get_lexeme())) {
        return m_values[token.get_lexeme()];
    }

    if (m_enclosing != nullptr) {
        return m_enclosing->get(token);
    }

    ErrorHandler::error(m_file_name, token.get_line(), "variable does not exist");
    throw "exception";
}

jl::Value& jl::Environment::get_at(Token& name, int depth)
{
    return ancestor(depth)->m_values[name.get_lexeme()];
}

void jl::Environment::assign(Token& token, Value& value)
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

void jl::Environment::assign(Token& token, Value&& value)
{
    if (m_values.contains(token.get_lexeme())) {
        m_values[token.get_lexeme()] = std::move(value);
        return;
    }

    if (m_enclosing != nullptr) {
        m_enclosing->assign(token, std::move(value));
        return;
    }

    ErrorHandler::error(m_file_name, token.get_line(), "Udefined variable");
    throw "exception";
}

void jl::Environment::assign_at(Token& token, Value& value, int depth)
{
    ancestor(depth)->m_values[token.get_lexeme()] = value;
}

jl::Environment* jl::Environment::ancestor(int depth)
{
    Environment* env = this;

    for (int i = 0; i < depth; i++) {
        env = env->m_enclosing;
    }

    return env;
}
