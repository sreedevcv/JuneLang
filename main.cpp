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
        fun get() [
            return {"332", "100"};
        ]

        var list = {3, 5, 1, 2, 6, {30, -5, {100}}, 9, 8, 4, 7};

        for (var a: list) [
            print a;
            a = 5;
        ]

        var sum = 0;
        for (var a: get()) [
            sum += int(a);
        ]
        print "Sum of " + str(get()) + " is " + str(sum);
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
