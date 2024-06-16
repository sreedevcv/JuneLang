#include "Callable.hpp"

jl::FunctionCallable::FunctionCallable(FuncStmt* declaration, Environment* closure)
    : m_declaration(declaration)
    , m_closure(closure)
{
}

jl::Value jl::FunctionCallable::call(Interpreter* interpreter, std::vector<Value>& arguments)
{
    Environment* env = new Environment(m_closure);

    for (int i = 0; i < m_declaration->m_params.size(); i++) {
        env->define(m_declaration->m_params[i]->get_lexeme(), arguments[i]);
    }

    try {
        interpreter->execute_block(m_declaration->m_body, env);
    } catch (Value value) {
        // To prevent the env from deleting the enclosing environment (which might still be needed) when it goes out of scope
        env->m_enclosing = nullptr;
        return value;
    }

    // To prevent the env from deleting the enclosing environment (which might still be needed) when it goes out of scope
    env->m_enclosing = nullptr;
    return '\0';
}

int jl::FunctionCallable::arity()
{
    return m_declaration->m_params.size();    return m_declaration->m_params.size();

}