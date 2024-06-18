#include "Callable.hpp"
#include "Value.hpp"

#include "ErrorHandler.hpp"

// --------------------------------------------------------------------------------
// -----------------------------FunctionCallable-----------------------------------
// --------------------------------------------------------------------------------

jl::FunctionCallable::FunctionCallable(FuncStmt* declaration, Environment* closure, bool is_initalizer)
    : m_declaration(declaration)
    , m_closure(closure)
    , m_is_initializer(is_initalizer)
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
        if (m_is_initializer) {
            return m_closure->get_at("self", 0);
        }
        return value;
    }

    // // To prevent the env from deleting the enclosing environment (which might still be needed) when it goes out of scope
    // env->m_enclosing = nullptr;
    if (m_is_initializer) {
        std::string self = "self";
        return m_closure->get_at("self", 0);
    }
    
    return '\0';
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
    Environment* env = new Environment(m_closure);
    env->define("self", instance);
    return new FunctionCallable(m_declaration, env, m_is_initializer);
}

// --------------------------------------------------------------------------------
// ----------------------------ToIntNativeFunction---------------------------------
// --------------------------------------------------------------------------------

// TODO::Take an optional line_no as argument in Callable::call so that error handler can print the line number
jl::Value jl::ToIntNativeFunction::call(Interpreter* interpreter, std::vector<Value>& arguments)
{
    Value& not_int = arguments[0];

    if (is_int(not_int)) {
        return not_int;
    } else if (is_float(not_int)) {
        return static_cast<int>(std::get<double>(not_int));
    } else if (is_bool(not_int)) {
        return std::get<bool>(not_int) ? 1 : 0;
    } else if (is_null(not_int)) {
        return 0;
    } else if (is_string(not_int)) {
        try {
            int num = std::stoi(std::get<std::string>(not_int));
            return num;
        } catch (...) {
            ErrorHandler::error(interpreter->m_file_name, "interpreting", "native function int()", 0, "Conversion cannot be performed on an invalid string", 0);
            throw "runtime-error";
            // return 0;
        }
    } else {
        ErrorHandler::error(interpreter->m_file_name, "interpreting", "native function int()", 0, "Conversion cannot be performed on a callable", 0);
        throw "runtime-error";
    }
}

int jl::ToIntNativeFunction::arity()
{
    return 1;
}

std::string jl::ToIntNativeFunction::to_string()
{
    return "<native fn: int>";
}

// --------------------------------------------------------------------------------
// -------------------------------ClassCallable------------------------------------
// --------------------------------------------------------------------------------

jl::ClassCallable::ClassCallable(std::string& name, std::map<std::string, FunctionCallable*>& methods)
    : m_name(name)
    , m_methods(methods)
{
}

jl::Value jl::ClassCallable::call(Interpreter* interpreter, std::vector<Value>& arguments)
{
    Instance* instance = new Instance(this);
    std::string init_name = "init";
    FunctionCallable* initializer = find_method(init_name);
    if (initializer != nullptr) {
        initializer->bind(instance)->call(interpreter, arguments);
    }
    return instance;
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
    return nullptr;
}

// --------------------------------------------------------------------------------
// -------------------------------Instance------------------------------------
// --------------------------------------------------------------------------------

jl::Instance::Instance(ClassCallable* class_callable)
    : m_class(class_callable)
{
}

jl::Value jl::Instance::get(Token& name)
{
    if (m_fields.contains(name.get_lexeme())) {
        return m_fields[name.get_lexeme()];
    }

    FunctionCallable* method = m_class->find_method(name.get_lexeme());
    if (method != nullptr) {
        Callable* method_instance = method->bind(this);
        return method_instance;
    }

    std::string fname = "unknown";
    ErrorHandler::error(fname, "interpreting", "field access", name.get_line(), "No such field exists for the class instance", 0);
    throw "runtime-exception";
}

void jl::Instance::set(Token& name, Value& value)
{
    m_fields[name.get_lexeme()] = value;
}

std::string jl::Instance::to_string()
{
    return m_class->to_string() + " instance";
}
