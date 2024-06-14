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
    R"(
        var a = 10;

        if 3 == 5 [
            a = a + 1;
        ]
        else [
            a = a -1;
            a = a * 2;
        ]

        print a;
    )");
    // jl::Lexer lexer("\"hello \" + \"hai\"");
    // lexer.scan();

    if (!jl::ErrorHandler::has_error()) {
        lexer.scan();
        auto tokens = lexer.get_tokens();
        jl::Parser parser(tokens);
        auto stmts = parser.parseStatements();

        jl::Token::Value v;
        jl::Interpreter interpreter;
        interpreter.interpret(stmts);
        // std::cout << interpreter.stringify(v) << "\n";
    }
}