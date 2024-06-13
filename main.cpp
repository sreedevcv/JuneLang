#include <iostream>

#include "ErrorHandler.hpp"
#include "Expr.hpp"
#include "Interpreter.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"

int main()
{
    // std::cout << "Hello World\n";

    jl::Lexer lexer(
    R"(var a = 1.0
        var b
        print a + b)");
    // jl::Lexer lexer("\"hello \" + \"hai\"");
    // lexer.scan();

    if (!jl::ErrorHandler::has_error()) {
        lexer.scan();
        auto tokens = lexer.get_tokens();
        jl::Parser parser(tokens);
        auto toks = parser.parseStatements();

        jl::Token::Value v;
        jl::Interpreter interpreter;
        interpreter.interpret(toks);
        // std::cout << interpreter.stringify(v) << "\n";
    }
}