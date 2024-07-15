#include <iostream>

#include "ErrorHandler.hpp"
#include "Expr.hpp"
#include "Interpreter.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "Resolver.hpp"

// #include "Editor.hpp"

int main()
{
    // jed::Editor editor;
    // editor.start();

    jl::Lexer lexer(
        R"(
        // var a = {3, 5, 1, 2, 6, 9, 8, 2+2, 7};

        // fun bubbleSort(list, size) [
        //     for (var i = 0; i < size - 1; i += 1) [
        //         for (var j = 0; j < size - i - 1; j += 1) [
        //             if (list[j] > list[j + 1]) [
        //                 var temp = list[j];
        //                 list[j] = list[j + 1];
        //                 list[j + 1] = temp;
        //             ]
        //         ]
        //     ]
        // ]

        // bubbleSort(a, 9);

        // print a;

        // var b = {};
        // print b;

        // print len(a);
        var b = 11;
        var a = {};
        append(a, b);
        print a;
        print b;
        print len(a);

    )");

    std::string file_name = "test";
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

    jl::Interpreter interpreter(arena, file_name, 1000*1000);
    
    jl::Resolver resolver(interpreter, file_name);
    resolver.resolve(stmts);
    
    if (jl::ErrorHandler::has_error()) {
        return 1;
    }

    interpreter.interpret(stmts);

    std::cout << "Ended" << std::endl;
}

/*


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
