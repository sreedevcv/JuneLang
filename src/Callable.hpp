#pragma once

// #include "Token.hpp"
#include "Value.hpp"
#include "Interpreter.hpp"

namespace jl {

class Callable {
public:
    virtual Value call(Interpreter *interpreter, std::vector<Value>& arguments) = 0;
    virtual int arity() = 0;
};

class Function: public Callable {
public:
    Function(FuncStmt* declaration);
    virtual ~Function() = default;

    virtual Value call(Interpreter *interpreter, std::vector<Value>& arguments) override;
    virtual int arity() override;
    
private:
    FuncStmt* m_declaration;
};

} // namespace jl
