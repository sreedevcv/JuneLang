#include "Callable.hpp"

jl::Function::Function(FuncStmt* declaration)
    : m_declaration(declaration)
{
}

jl::Value jl::Function::call(Interpreter* interpreter, std::vector<Value>& arguments)
{
    Environment* env = new Environment(interpreter->m_global_env);

    for (int i = 0; i < m_declaration->m_params.size(); i++) {
        env->define(m_declaration->m_params[i]->get_lexeme(), arguments[i]);
    }

    interpreter->execute_block(m_declaration->m_body, env);

    env->m_enclosing = nullptr;
}

int jl::Function::arity() 
{
    return m_declaration->m_params.size();
}