#pragma once

#include "Callable.hpp"

namespace jl {
class ToIntNativeFunction : public Callable {
public:
    ToIntNativeFunction() = default;
    virtual ~ToIntNativeFunction() = default;

    virtual Value* call(Interpreter* interpreter, std::vector<Value*>& arguments) override;
    virtual int arity() override;
    virtual std::string to_string() override;

    std::string m_name = "int";
};

class ToStrNativeFunction : public Callable {
public:
    ToStrNativeFunction() = default;
    virtual ~ToStrNativeFunction() = default;

    virtual Value* call(Interpreter* interpreter, std::vector<Value*>& arguments) override;
    virtual int arity() override;
    virtual std::string to_string() override;

    std::string m_name = "str";
};

#define NATIVE_FUCTION(CLASS_NAME, FUNC_NAME, FUNC, ARITY, ...)                                     \
    class CLASS_NAME##NativeFunction : public Callable {                                            \
    public:                                                                                         \
        CLASS_NAME##NativeFunction() = default;                                                     \
        ~CLASS_NAME##NativeFunction() = default;                                                    \
        std::string m_name = #FUNC_NAME;                                                            \
        inline virtual Value* call(Interpreter* interpreter, std::vector<Value*>& arguments) override \
        {                                                                                           \
            return FUNC(__VA_ARGS__);                                                               \
        }                                                                                           \
        inline virtual int arity() override                                                         \
        {                                                                                           \
            return ARITY;                                                                           \
        }                                                                                           \
        inline virtual std::string to_string() override                                             \
        {                                                                                           \
            return "<native fn: " #FUNC_NAME ">";                                                   \
        }                                                                                           \
    }

Value* jlist_get_len(Interpreter* interpreter, std::string& file_name, Value* jlist);
Value* jlist_push_back(Interpreter* interpreter, Value* jlist, Value* appending_value);
Value* jlist_pop_back(Interpreter* interpreter, Value* jlist);
Value* jlist_clear(Interpreter* interpreter, Value* jlist);

NATIVE_FUCTION(GetLen, len, jlist_get_len, 1, interpreter, interpreter->m_file_name, arguments[0]);
NATIVE_FUCTION(Append, push_back, jlist_push_back, 2, interpreter, arguments[0], arguments[1]);
NATIVE_FUCTION(RemoveLast, pop_back, jlist_pop_back, 1, interpreter, arguments[0]);
NATIVE_FUCTION(ClearList, clear, jlist_clear, 1, interpreter, arguments[0]);

} // namespace jl
