#pragma once

#include "Interpreter.hpp"
#include "Value.hpp"

namespace jl {


class Callable {
public:
    virtual Value call(Interpreter* interpreter, std::vector<Value>& arguments) = 0;
    virtual int arity() = 0;
    virtual std::string to_string() = 0;
    ~Callable() = default;
};

class FunctionCallable : public Callable {
public:
    FunctionCallable(FuncStmt* declaration, std::shared_ptr<Environment>& closure, bool is_initalizer);
    virtual ~FunctionCallable() = default;

    virtual Value call(Interpreter* interpreter, std::vector<Value>& arguments) override;
    virtual int arity() override;
    virtual std::string to_string() override;

    FunctionCallable* bind(Instance* instance);
private:
    FuncStmt* m_declaration;
    std::shared_ptr<Environment> m_closure;
    bool m_is_initializer = false;
};

class ClassCallable: public Callable {
public:
    ClassCallable(std::string& name, ClassCallable* super_class, std::map<std::string, FunctionCallable*>& methods);
    virtual ~ClassCallable();

    virtual Value call(Interpreter* interpreter, std::vector<Value>& arguments) override;
    virtual int arity() override;
    virtual std::string to_string() override;

    FunctionCallable* find_method(std::string& name);

private:
    std::string m_name;
    std::map<std::string, FunctionCallable*> m_methods;
    ClassCallable* m_super_class;
};

class Instance {
public:
    Instance(ClassCallable* class_callable);
    ~Instance();

    Value get(Token& name);
    void set(Token& name, Value& value);
    std::string to_string();
private:
    ClassCallable* m_class;
    std::map<std::string, Value> m_fields;
};

class ToIntNativeFunction : public Callable {
public:
    ToIntNativeFunction() = default;
    virtual ~ToIntNativeFunction() = default;

    virtual Value call(Interpreter* interpreter, std::vector<Value>& arguments) override;
    virtual int arity() override;
    virtual std::string to_string() override;

    std::string m_name = "int";
};

} // namespace jl
