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

            var a = "global a"
            var b = "global b"
            var c = "global c"
            [
            var a = "outer a"
            var b = "outer b"
            [
                var a = "inner a"
                print a
                print b
                print c
            ]
            print a
            print b
            print c
            ]
            print a
            print b
            print c
            print a + "-" + b + "-" + c

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