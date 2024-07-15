#include "NativeFunctions.hpp"

#include "ErrorHandler.hpp"

jl::Value jl::ToStrNativeFunction::call(Interpreter* interpreter, std::vector<Value>& arguments)
{
    return interpreter->stringify(arguments[0]);
}

int jl::ToStrNativeFunction::arity()
{
    return 1;
}

std::string jl::ToStrNativeFunction::to_string()
{
    return "<native fn: str>";
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

jl::Value jl::get_len(std::string& file_name, Value& jlist)
{
    if (is_jlist(jlist)) {
        return static_cast<int>(std::get<std::vector<Expr*>*>(jlist)->size());
    } else {
        ErrorHandler::error(file_name, "interpreting", "native function len()", 0, "Attempted to use len() on a non-list", 0);
        return 0;
    }
}

jl::Value jl::append(Interpreter* interpreter, Value& jlist, Value& appending_value)
{
    if (is_jlist(jlist)) {
        auto list = std::get<std::vector<Expr*>*>(jlist);
        Value* v = interpreter->m_internal_arena.allocate<Value>(std::move(appending_value));      // Wastefull copying
        list->push_back(interpreter->m_internal_arena.allocate<Literal>(v));
        // interpreter.ev
        return list;
    } else {
        ErrorHandler::error(interpreter->m_file_name, "interpreting", "native function len()", 0, "Attempted to use append() on a non-list", 0);
        return 0;
    }
}
