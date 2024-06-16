#pragma once

#include "Value.hpp"
#include "Interpreter.hpp"

namespace jl {

class Callable {
public:
    virtual Value call(Interpreter *interpreter, std::vector<Value>& arguments) = 0;
    virtual int arity() = 0;
};


class FunctionCallable: public Callable {
public:
    FunctionCallable(FuncStmt* declaration, Environment* closure);
    virtual ~FunctionCallable() = default;

    virtual Value call(Interpreter *interpreter, std::vector<Value>& arguments) override;
    virtual int arity() override;
    
private:
    FuncStmt* m_declaration;
    Environment* m_closure;
};

} // namespace jl
