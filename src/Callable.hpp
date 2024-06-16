#pragma once

#include "Interpreter.hpp"
#include "Value.hpp"

namespace jl {

class Callable {
public:
    virtual Value call(Interpreter* interpreter, std::vector<Value>& arguments) = 0;
    virtual int arity() = 0;
    virtual std::string to_string() = 0;
};

class FunctionCallable : public Callable {
public:
    FunctionCallable(FuncStmt* declaration, Environment* closure);
    virtual ~FunctionCallable() = default;

    virtual Value call(Interpreter* interpreter, std::vector<Value>& arguments) override;
    virtual int arity() override;
    virtual std::string to_string() override;

private:
    FuncStmt* m_declaration;
    Environment* m_closure;
};

class ToIntNativeFunction : public Callable {
public:
    ToIntNativeFunction() = default;
    ~ToIntNativeFunction() = default;

    virtual Value call(Interpreter* interpreter, std::vector<Value>& arguments) override;
    virtual int arity() override;
    virtual std::string to_string() override;

    std::string m_name = "int";
};

} // namespace jl
