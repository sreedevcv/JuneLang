#include <iostream>

#include "ErrorHandler.hpp"
#include "Expr.hpp"
#include "Interpreter.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "Resolver.hpp"

// #include "Editor.hpp"

int main(int argc, char const *argv[])
{
    // jed::Editor editor;
    // editor.start();

    // jl::Lexer lexer(
        R"(
        class Elist [
            init() [
                self.list = {};
            ]

            push(ele) [
                push_back(self.list, ele);
            ]

            push_list(list) [
                for (var ele: list) [
                    push_back(self.list, ele);
                ]
            ]

            get(index) [
                return self.list[index];
            ]

            set(index, ele) [
                self.list[index] = ele;
            ]

            contains(ele) [
                for (var item: self.list) [
                    if (ele == item) [
                        return true;
                    ]
                ]
                return false;
            ]

            remove(ele) [
        for (var i = 0; i < len(self.list); i += 1) [
            if (self.list[i] == ele) [
                for (var j = i + 1; j < len(self.list) - 1; j += 1) [
                    self.list[j - 1] = self.list[j];
                ]
                pop_back(self.list);
            ]
        ]
    ]
        ]    

        var l1 = {1, 2, 3};
        var l2 = {5, 6, 7, 8};
        var elist = Elist();

        elist.push_list(l1);
        elist.push(4);
        elist.push_list(l2);
        elist.pop();
        print elist.list;

        print elist.contains(4);
        print elist.contains(-10);
    )";
    
    std::string file_name = "examples/EList.jun";

    if  (argc == 2) {
        file_name = argv[1];
    }

    jl::Lexer lexer(file_name);
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
