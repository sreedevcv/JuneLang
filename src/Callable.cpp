#include "Callable.hpp"

jl::FunctionCallable::FunctionCallable(FuncStmt* declaration)
    : m_declaration(declaration)
{
}

jl::Value jl::FunctionCallable::call(Interpreter* interpreter, std::vector<Value>& arguments)
{
    Environment* env = new Environment(interpreter->m_global_env);

    for (int i = 0; i < m_declaration->m_params.size(); i++) {
        env->define(m_declaration->m_params[i]->get_lexeme(), arguments[i]);
    }

    try {
        interpreter->execute_block(m_declaration->m_body, env);
    } catch (Value value) {
        // To prevent the env from deleting the enclosing global environment when it goes out of scope
        env->m_enclosing = nullptr;
        delete env;
        return value;
    }

    // To prevent the env from deleting the enclosing global environment when it goes out of scope
    env->m_enclosing = nullptr;
    delete env;
    return '\0';
}

int jl::FunctionCallable::arity()
{
    return m_declaration->m_params.size();
}