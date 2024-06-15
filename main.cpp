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
        fun add(a, b, c) [
            print a + b + c;
        ]

        add(1, 2, 3);
        add(4, 5, 6);
        
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