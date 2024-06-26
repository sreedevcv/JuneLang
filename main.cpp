#include <iostream>

#include "ErrorHandler.hpp"
#include "Expr.hpp"
#include "Interpreter.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "Resolver.hpp"

#include "Editor.hpp"

#include "FileHandler.hpp"

int main()
{
    // jed::FileHandler fh;
    // std::string file="main.cpp";
    // std::cout << fh.open_and_read(file);
    jed::Editor editor;
    editor.start();


    // jl::Lexer lexer(
    //     R"(
    //     var a = 10;
    //     print a;

    //     fun temp(x, y)
    //     [
    //         for var i=x; i < y; i = i+1;
    //         [
    //             print i;
    //         ]
    //     ]

    //     temp(10, 20);

    // )");

    // std::string file_name = "test";
    // lexer.scan();

    // if (jl::ErrorHandler::has_error()) {
    //     return 1;
    // }

    // auto tokens = lexer.get_tokens();
    // jl::Parser parser(tokens, file_name);
    // auto stmts = parser.parseStatements();

    // if (jl::ErrorHandler::has_error()) {
    //     return 1;
    // }

    // jl::Interpreter interpreter(file_name);

    // jl::Resolver resolver(interpreter, file_name);
    // resolver.resolve(stmts);

    // if (jl::ErrorHandler::has_error()) {
    //     return 1;
    // }

    // jl::Value v;
    // interpreter.interpret(stmts);

    // for (auto stmt: stmts) {
    //     delete stmt;
    // }

    std::cout << "Ended" << std::endl;
}

/*

        fun fib(n) [
            if n <= 1 return n;
            return fib(n - 2) + fib(n - 1);
        ]

        for var i = 0; i < 20; i = i + 1; [
            print fib(i);
        ]


        class Thing [
            getCallback() [
                fun localFunction() [
                    print self;
                ]

                return localFunction;
            ]
        ]

        var callback = Thing().getCallback();
        callback();


                class Egotist [
            speak() [
                print self;
            ]
        ]

        var method = Egotist().speak;
        method();

        class Cake [
            taste() [
                var adjective = "delicious";
                print "The " + self.flavor + " cake is " + adjective + "!";
            ]
        ]

        var cake = Cake();
        cake.flavor = "German chocolate";
        cake.taste();

*/