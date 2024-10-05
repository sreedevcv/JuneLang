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
    // for (auto& [key, value]: m_values) {
    //     if (is_callable(value)) {
    //         delete std::get<Callable*>(value);
    //     } else if (is_instance(value)) {
    //         delete std::get<Instance*>(value);
    //     }
    // }
}

void jl::Environment::define(const std::string& name, const JlValue& value)
{
    if (!m_values.contains(name)) {
        m_values[name] = value;
    } else {
        ErrorHandler::error(m_file_name, 0, "variable already exists");
        throw "exception";
    }
}

jl::JlValue& jl::Environment::get(Token& token)
{
    if (m_values.contains(token.get_lexeme())) {
        return m_values[token.get_lexeme()];
    }

    if (m_enclosing != nullptr) {
        return m_enclosing->get(token);
    }

    std::string msg = "variable does not exist: " + token.get_lexeme();
    ErrorHandler::error(m_file_name, token.get_line(), msg.c_str());
    throw "exception";
}

jl::JlValue& jl::Environment::get_at(Token& name, int depth)
{
    Environment* env = ancestor(depth);
    return env->m_values[name.get_lexeme()];
}

jl::JlValue& jl::Environment::get_at(std::string& name, int depth)
{
    Environment* env = ancestor(depth);
    return env->m_values[name];
}

void jl::Environment::assign(Token& token, JlValue& value)
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

void jl::Environment::assign(Token& token, JlValue&& value)
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

void jl::Environment::assign_at(Token& token, JlValue& value, int depth)
{
    ancestor(depth)->m_values[token.get_lexeme()] = value;
}

jl::Environment* jl::Environment::ancestor(int depth)
{
    Environment* env = this;

    for (int i = 0; i < depth; i++) {
        env = (env->m_enclosing);
    }

    return env;
}

// 2544 byte(s) leaked in 34 allocation(s).