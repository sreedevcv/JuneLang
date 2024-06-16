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
        fun makeCounter() [
            var i = 0;
            fun count() [
                i = i + 1;
                print i;
            ]

            return count;
        ]

        var counter = makeCounter();
        counter(); // "1".
        counter(); // "2".
        
    )");

    std::string file_name = "test";
    lexer.scan();

    if (!jl::ErrorHandler::has_error()) {
        auto tokens = lexer.get_tokens();
        jl::Parser parser(tokens, file_name);
        auto stmts = parser.parseStatements();

        jl::Value v;
        jl::Interpreter interpreter(file_name);
        interpreter.interpret(stmts);
    }

}

/*

        fun fib(n) [
            if n <= 1 return n;
            return fib(n - 2) + fib(n - 1);
        ]

        for var i = 0; i < 20; i = i + 1; [
            print fib(i);
        ]


*/