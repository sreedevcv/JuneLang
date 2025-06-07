#include "Chunk.hpp"
#include "CodeGenerator.hpp"
#include "ErrorHandler.hpp"
#include "Interpreter.hpp"
#include "Lexer.hpp"
#include "OpCode.hpp"
#include "Parser.hpp"
#include "Resolver.hpp"
#include "Value.hpp"
#include <print>

int main(int argc, char const* argv[])
{
    // jed::Editor editor;
    // editor.start();

    // jl::Lexer lexer(
    //     R"(
    //     var a = {3, 5, 1, 2, 6, 9, 8, 4, 7};

    //    fun bubbleSort(list, size) [
    //        for (var i = 0; i < size - 1; i += 1) [
    //            for (var j = 0; j < size - i - 1; j += 1) [
    //                if (list[j] > list[j + 1]) [
    //                    var temp = list[j];
    //                    list[j] = list[j + 1];
    //                    list[j + 1] = temp;
    //                ]
    //            ]
    //        ]
    //    ]

    //    bubbleSort(a, 9);

    //    for (var i = 0; i < 9; i += 1) [
    //        print a[i];
    //    ]
    //    )");

    // std::string file_name = "examples/EList.jun";

    // lexer.scan();

    // jl::Arena arena(1000 * 1000);

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

    // interpreter.interpret(stmts);

    // jl::CodeGenerator generator(file_name);
    // generator.generate(stmts);

    jl::Chunk chunk;
    auto idx = chunk.add_constant(jl::Value { 1 });
    chunk.add_opcode(jl::OpCode::CONSTANT, 0);
    chunk.add_opcode((jl::OpCode)idx, 0);
    chunk.add_opcode(jl::OpCode::RETURN, 1);
    std::println("{}", chunk.disassemble());

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