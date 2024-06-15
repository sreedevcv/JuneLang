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
        // var index = 10;
        // var sum = 0;

        // while index > 0
        // [
        //     sum = sum + index;
        //     index = index - 1;
        // ]

        var a = 0;
        var temp;

        for var b = 1; a < 10000; b = temp + b; 
        [
            print a;
            temp = a;
            a = b;
        ]

        print "Hello";

        fun hello()
        [
        ]

        hello();

    )");
    // jl::Lexer lexer("\"hello \" + \"hai\"");
    // lexer.scan();

    std::string file_name = "test";

    if (!jl::ErrorHandler::has_error()) {
        lexer.scan();
        auto tokens = lexer.get_tokens();
        jl::Parser parser(tokens, file_name);
        auto stmts = parser.parseStatements();

        jl::Value v;
        jl::Interpreter interpreter(file_name);
        interpreter.interpret(stmts);
        // std::cout << interpreter.stringify(v) << "\n";
    }
}