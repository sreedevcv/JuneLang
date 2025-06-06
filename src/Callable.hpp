#pragma once

#include "Environment.hpp"
#include "Interpreter.hpp"
#include "Ref.hpp"
#include "Value.hpp"
#include "Stmt.hpp"

namespace jl {

class MemoryPool;
class Interpreter;

class Callable : public Ref {
public:
    virtual Value* call(Interpreter* interpreter, std::vector<Value*>& arguments) = 0;
    virtual int arity() = 0;
    virtual std::string to_string() = 0;
    virtual ~Callable() = default;
};

class FunctionCallable : public Callable {
public:
    FunctionCallable(Interpreter* interpreter, Environment* closure, FuncStmt* declaration, bool is_initalizer);
    virtual ~FunctionCallable() = default;

    virtual Value* call(Interpreter* interpreter, std::vector<Value*>& arguments) override;
    virtual int arity() override;
    virtual std::string to_string() override;

    FunctionCallable* bind(Instance* instance);

private:
    Interpreter* m_interpreter;
    Environment* m_closure;
    FuncStmt* m_declaration;
    bool m_is_initializer = false;

    friend class MemoryPool;
};

class ClassCallable : public Callable {
public:
    ClassCallable(std::string& name, ClassCallable* super_class, std::map<std::string, FunctionCallable*>& methods);
    virtual ~ClassCallable();

    virtual Value* call(Interpreter* interpreter, std::vector<Value*>& arguments) override;
    virtual int arity() override;
    virtual std::string to_string() override;

    FunctionCallable* find_method(std::string& name);

private:
    std::string m_name;
    std::map<std::string, FunctionCallable*> m_methods;
    ClassCallable* m_super_class;

    friend class MemoryPool;
};

class Instance : public Ref {
public:
    Instance(ClassCallable* class_callable);
    virtual ~Instance();

    Value* get(Token& name, Interpreter* interpreter);
    void set(Token& name, Value* value);
    std::string to_string();

private:
    ClassCallable* m_class;
    std::map<std::string, Value*> m_fields;

    friend class MemoryPool;
};

} // namespace jl
