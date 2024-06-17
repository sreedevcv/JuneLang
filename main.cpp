#include <iostream>

#include "ErrorHandler.hpp"
#include "Expr.hpp"
#include "Interpreter.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "Resolver.hpp"

int main()
{
    // std::cout << "Hello World\n";

    jl::Lexer lexer(
        R"(
        // fun makeCounter() [
        //     var i = 0;
        //     fun count() [
        //         i = i + 1;
        //         print i;
        //     ]

        //     return count;
        // ]

        // var counter = makeCounter();
        // counter(); // "1".
        // counter(); // "2".

        fun fib(n) [
            if n <= 1 return n;
            return fib(n - 2) + fib(n - 1);
        ]

        // for var i = 0; i < 20; i = i + 1; [
        //     print fib(i);
        // ]

        // print fib;
        // print int;
        // print int(2);    
        // print int(10.5);    
        // print int("545");    
        // print int("-23233.4324");   
        // print int(true); 
        // print int(false); 
        // print int(null); 
        // print int("hello");
        // print int(fib); 

        var a = "global";
        [
            fun showA() [
                print a;
            ]

            showA();
            var a = "block";
            showA();
        ]

        fun bad() [
            var a = "first";
            var a = "second";
        ]

        var a = "outer";
        [
            var a = a;
        ]

        return 1;

    )");

    std::string file_name = "test";
    lexer.scan();

    if (jl::ErrorHandler::has_error()) {
        return 1;
    }

    auto tokens = lexer.get_tokens();
    jl::Parser parser(tokens, file_name);
    auto stmts = parser.parseStatements();

    if (jl::ErrorHandler::has_error()) {
        return 1;
    }

    jl::Interpreter interpreter(file_name);

    jl::Resolver resolver(interpreter, file_name);
    resolver.resolve(stmts);

    if (jl::ErrorHandler::has_error()) {
        return 1;
    }


    jl::Value v;
    interpreter.interpret(stmts);
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