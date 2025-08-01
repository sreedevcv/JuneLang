#include "NativeFunctions.hpp"

#include "ErrorHandler.hpp"
#include "Value.hpp"

jl::Value* jl::ToStrNativeFunction::call(Interpreter* interpreter, std::vector<Value*>& arguments)
{
    return interpreter->m_gc.allocate<Value>(stringify(arguments[0]));
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
jl::Value* jl::ToIntNativeFunction::call(Interpreter* interpreter, std::vector<Value*>& arguments)
{
    Value* not_int = arguments[0];

    if (is::_int(*not_int)) {
        return not_int;
    } else if (is::_float(*not_int)) {
        int retval = static_cast<int>(std::get<double>(not_int->get()));
        return interpreter->m_gc.allocate<Value>(retval);
    } else if (is::_bool(*not_int)) {
        return std::get<bool>(not_int->get()) ? interpreter->m_gc.allocate<Value>(1) : interpreter->m_gc.allocate<Value>(0);
    } else if (is::_null(*not_int)) {
        return interpreter->m_gc.allocate<Value>(0);
    } else if (is::_str(*not_int)) {
        try {
            int num = std::stoi(std::get<std::string>(not_int->get()));
            return interpreter->m_gc.allocate<Value>(num);
        } catch (...) {
            ErrorHandler::error(interpreter->m_file_name, "interpreting", "native function int()", 0, "Conversion cannot be performed on an invalid string", 0);
            throw "runtime-error";
            // return 0
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

jl::Value* jl::jlist_get_len(Interpreter* interpreter, std::string& file_name, Value* jlist)
{
    if (is::_list(*jlist)) {
        int size = static_cast<int>(std::get<std::vector<Expr*>>(jlist->get()).size());
        return interpreter->m_gc.allocate<Value>(size);
    } else {
        ErrorHandler::error(file_name, "interpreting", "native function len()", 0, "Attempted to use len() on a non-list", 0);
        return interpreter->m_gc.allocate<Value>(-1);
    }
}

jl::Value* jl::jlist_push_back(Interpreter* interpreter, Value* jlist, Value* appending_value)
{
    if (is::_list(*jlist)) {
        auto& list = std::get<std::vector<Expr*>>(jlist->get());
        list.push_back(interpreter->m_gc.allocate<Literal>(appending_value));
        return interpreter->m_gc.allocate<Value>(Null {});
    } else {
        ErrorHandler::error(interpreter->m_file_name, "interpreting", "native function len()", 0, "Attempted to use push_back() on a non-list", 0);
        throw "runtime-error";
    }
}

jl::Value* jl::jlist_pop_back(Interpreter* interpreter, Value* jlist)
{
    if (is::_list(*jlist)) {
        auto& list = std::get<std::vector<Expr*>>(jlist->get());
        list.pop_back();
        return interpreter->m_gc.allocate<Value>(Null {});
    } else {
        ErrorHandler::error(interpreter->m_file_name, "interpreting", "native function len()", 0, "Attempted to use pop_back() on a non-list", 0);
        throw "runtime-error";
    }
}

jl::Value* jl::jlist_clear(Interpreter* interpreter, Value* jlist)
{
    if (is::_list(*jlist)) {
        auto& list = std::get<std::vector<Expr*>>(jlist->get());
        list.clear();
        return interpreter->m_gc.allocate<Value>(Null {});
    } else {
        ErrorHandler::error(interpreter->m_file_name, "interpreting", "native function len()", 0, "Attempted to use clear() on a non-list", 0);
        throw "runtime-error";
    }
}
