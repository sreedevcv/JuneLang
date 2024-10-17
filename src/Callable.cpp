#include "Callable.hpp"
#include "Environment.hpp"
#include "Value.hpp"

#include "ErrorHandler.hpp"

// --------------------------------------------------------------------------------
// -----------------------------FunctionCallable-----------------------------------
// --------------------------------------------------------------------------------

jl::FunctionCallable::FunctionCallable(Interpreter* interpreter, Environment* closure, FuncStmt* declaration, bool is_initalizer)
    : m_interpreter(interpreter)
    , m_closure(closure)
    , m_declaration(declaration)
    , m_is_initializer(is_initalizer)
{
}

jl::JlValue* jl::FunctionCallable::call(Interpreter* interpreter, std::vector<JlValue*>& arguments)
{
    // TODO::Do I actually need this environment to persist???
    Environment* env = m_interpreter->m_gc.allocate<Environment>(m_closure);

    for (int i = 0; i < m_declaration->m_params.size(); i++) {
        env->define(m_declaration->m_params[i]->get_lexeme(), arguments[i]);
    }

    try {
        interpreter->execute_block(m_declaration->m_body, env);
    } catch (JlValue* value) {
        if (m_is_initializer) {
            return m_closure->get_at(Token::global_this_lexeme, 0);
        }
        return value;
    }

    if (m_is_initializer) {
        return m_closure->get_at(Token::global_this_lexeme, 0);
    }

    return interpreter->m_gc.allocate<JlNull>();
}

int jl::FunctionCallable::arity()
{
    return m_declaration->m_params.size();
}

std::string jl::FunctionCallable::to_string()
{
    return "<fn: " + m_declaration->m_name.get_lexeme() + ">";
}

jl::FunctionCallable* jl::FunctionCallable::bind(Instance* instance)
{
    // This env will be cleaned up by the FunctionCallable
    Environment* env = m_interpreter->m_gc.allocate<Environment>(m_closure);
    env->define(Token::global_this_lexeme, m_interpreter->m_gc.allocate<JlObj>(instance));
    return m_interpreter->m_gc.allocate<FunctionCallable>(m_interpreter, env, m_declaration, m_is_initializer);
}

// --------------------------------------------------------------------------------
// -------------------------------ClassCallable------------------------------------
// --------------------------------------------------------------------------------

jl::ClassCallable::ClassCallable(std::string& name, ClassCallable* super_class, std::map<std::string, FunctionCallable*>& methods)
    : m_name(name)
    , m_super_class(super_class)
    , m_methods(methods)
{
}

jl::ClassCallable::~ClassCallable()
{
}

jl::JlValue* jl::ClassCallable::call(Interpreter* interpreter, std::vector<JlValue*>& arguments)
{
    Instance* instance = interpreter->m_gc.allocate<Instance>(this);
    std::string init_name = "init";
    FunctionCallable* initializer = find_method(init_name);
    if (initializer != nullptr) {
        initializer->bind(instance)->call(interpreter, arguments);
    }
    return interpreter->m_gc.allocate<JlObj>(instance);
}

int jl::ClassCallable::arity()
{
    std::string init_name = "init";
    FunctionCallable* initializer = find_method(init_name);
    return initializer == nullptr ? 0 : initializer->arity();
}

std::string jl::ClassCallable::to_string()
{
    return m_name;
}

jl::FunctionCallable* jl::ClassCallable::find_method(std::string& name)
{
    if (m_methods.contains(name)) {
        return m_methods[name];
    }

    if (m_super_class != nullptr) {
        return m_super_class->find_method(name);
    }
    return nullptr;
}

// --------------------------------------------------------------------------------
// -------------------------------Instance------------------------------------
// --------------------------------------------------------------------------------

jl::Instance::Instance(ClassCallable* class_callable)
    : m_class(class_callable)
{
}

jl::Instance::~Instance()
{
    // for (auto& [key, value]: m_fields) {
    //     if (is_callable(value)) {
    //         delete std::get<Callable*>(value);
    //     } else if (is_instance(value)) {
    //         delete std::get<Instance*>(value);
    //     }
    // }
}

jl::JlValue* jl::Instance::get(Token& name, Interpreter *interpreter)
{
    if (m_fields.contains(name.get_lexeme())) {
        return m_fields[name.get_lexeme()];
    }

    FunctionCallable* method = m_class->find_method(name.get_lexeme());
    if (method != nullptr) {
        Callable* method_instance = method->bind(this);
        return interpreter->m_gc.allocate<JlCallable>(method_instance);
    }

    std::string fname = "unknown";
    ErrorHandler::error(fname, "interpreting", "field access", name.get_line(), "No such field exists for the class instance", 0);
    throw "runtime-exception";
}

void jl::Instance::set(Token& name, JlValue* value)
{
    m_fields[name.get_lexeme()] = value;
}

std::string jl::Instance::to_string()
{
    return m_class->to_string() + " instance";
}
