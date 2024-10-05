#include "NativeFunctions.hpp"

#include "ErrorHandler.hpp"
#include "Value.hpp"

jl::JlValue jl::ToStrNativeFunction::call(Interpreter* interpreter, std::vector<JlValue>& arguments)
{
    return JlValue(interpreter->stringify(arguments[0]));
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
jl::JlValue jl::ToIntNativeFunction::call(Interpreter* interpreter, std::vector<JlValue>& arguments)
{
    JlValue& not_int = arguments[0];

    if (is_int(not_int)) {
        return not_int;
    } else if (is_float(not_int)) {
        int retval = static_cast<int>(std::get<double>(not_int.get()));
        return JlValue(retval);
    } else if (is_bool(not_int)) {
        return std::get<bool>(not_int.get()) ? JlValue(1) : JlValue(0);
    } else if (is_null(not_int)) {
        return JlValue(0);
    } else if (is_string(not_int)) {
        try {
            int num = std::stoi(std::get<std::string>(not_int.get()));
            return JlValue(num);
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

jl::JlValue jl::jlist_get_len(std::string& file_name, JlValue& jlist)
{
    if (is_jlist(jlist)) {
        int size = static_cast<int>(std::get<std::vector<Expr*>*>(jlist.get())->size());
        return JlValue(size);
    } else {
        ErrorHandler::error(file_name, "interpreting", "native function len()", 0, "Attempted to use len() on a non-list", 0);
        return JlValue(-1);
    }
}

jl::JlValue jl::jlist_push_back(Interpreter* interpreter, JlValue& jlist, JlValue& appending_value)
{
    if (is_jlist(jlist)) {
        auto list = std::get<std::vector<Expr*>*>(jlist.get());
        // JlValue* v = interpreter->m_internal_arena.allocate<JlValue>(std::move(appending_value)); // Wastefull copying
        list->push_back(interpreter->m_internal_arena.allocate<Literal>(appending_value));
        return JlValue(JNullType{});
    } else {
        ErrorHandler::error(interpreter->m_file_name, "interpreting", "native function len()", 0, "Attempted to use push_back() on a non-list", 0);
        throw "runtime-error";
    }
}

jl::JlValue jl::jlist_pop_back(Interpreter* interpreter, JlValue& jlist)
{
    if (is_jlist(jlist)) {
        auto list = std::get<std::vector<Expr*>*>(jlist.get());
        list->pop_back();
        return JlValue(JNullType{});
    } else {
        ErrorHandler::error(interpreter->m_file_name, "interpreting", "native function len()", 0, "Attempted to use pop_back() on a non-list", 0);
        throw "runtime-error";
    }
}

jl::JlValue jl::jlist_clear(Interpreter* interpreter, JlValue& jlist)
{
    if (is_jlist(jlist)) {
        auto list = std::get<std::vector<Expr*>*>(jlist.get());
        list->clear();
        return JlValue(JNullType{});
    } else {
        ErrorHandler::error(interpreter->m_file_name, "interpreting", "native function len()", 0, "Attempted to use clear() on a non-list", 0);
        throw "runtime-error";
    }
}
