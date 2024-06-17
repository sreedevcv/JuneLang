#include "Callable.hpp"
#include "Value.hpp"

#include "ErrorHandler.hpp"

// --------------------------------------------------------------------------------
// -----------------------------FunctionCallable-----------------------------------
// --------------------------------------------------------------------------------

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
    return m_declaration->m_params.size();
}

std::string jl::FunctionCallable::to_string()
{
    return "<fn: " + m_declaration->m_name.get_lexeme() + ">";
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
        } catch(...) {
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

jl::ClassCallable::ClassCallable(std::string& name)
    : m_name(name)
{
}

jl::Value jl::ClassCallable::call(Interpreter* interpreter, std::vector<Value>& arguments)
{
    Instance* instance = new Instance(this);
    return instance;
}

int jl::ClassCallable::arity()
{
    return 0;
}

std::string jl::ClassCallable::to_string()
{
    return m_name;
}

// --------------------------------------------------------------------------------
// -------------------------------Instance------------------------------------
// --------------------------------------------------------------------------------

jl::Instance::Instance()
{
}

jl::Instance::Instance(ClassCallable* class_callable)
    : m_class(class_callable)
{
}

std::string jl::Instance::to_string()
{
    return m_class->to_string() + " instance"; 
}
