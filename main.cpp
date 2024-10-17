#include "ErrorHandler.hpp"
#include "Interpreter.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "Resolver.hpp"

// #include "Editor.hpp"

int main(int argc, char const *argv[])
{
    // jed::Editor editor;
    // editor.start();

    jl::Lexer lexer(
        R"(
        fun fib(n) [
            if (n <= 1) return n;
            //var a = fib(n - 2);
            //var b = fib(n - 1);
            //return a + b;
            return fib(n - 2) + fib(n - 1);
        ]
       
         for (var i = 0; i < 10; i += 1) [
            print fib(i);
         ]
    )");
    
    std::string file_name = "examples/EList.jun";

    // if  (argc == 2) {
    //     file_name = argv[1];
    // }

    // jl::Lexer lexer(file_name);
    lexer.scan();

    jl::Arena arena(1000 * 1000);

    if (jl::ErrorHandler::has_error()) {
        return 1;
    }

    auto tokens = lexer.get_tokens();
    jl::Parser parser(arena, tokens, file_name);
    auto stmts = parser.parseStatements();

    if (jl::ErrorHandler::has_error()) {
        return 1;
    }

    jl::Interpreter interpreter(arena, file_name, 1000*1000*10*5);
    
    jl::Resolver resolver(interpreter, file_name);
    resolver.resolve(stmts);
    
    if (jl::ErrorHandler::has_error()) {
        return 1;
    }

    interpreter.interpret(stmts);
    return 0;
}

/*
        var a = {3, 5, 1, 2};

        for (var item: a) [
            if (item <= 1) [
                print "BREAKING";
                break;
            ]
        ]

        for (var i = 0; i < 9; i+=1) [
            print i;

            if (i == 5) [
                break;
            ]
        ]

        while (true) [
            break;
        ]

        for (var i = 0; i < 10; i+=1) [
            var line = "";

            for (var j = 0; j < 10; j+=1) [
                if (j > i) [
                    break;
                ]
                line += str(j) + " ";
            ]

            print line;
        ]

*/