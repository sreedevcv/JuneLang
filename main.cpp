#include "CodeGenerator.hpp"
#include "ErrorHandler.hpp"
#include "Flatten.hpp"
#include "Interpreter.hpp"
#include "Lexer.hpp"
#include "Operand.hpp"
#include "Parser.hpp"
#include "Resolver.hpp"
#include "VM.hpp"

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <print>
#include <string>

int main(int argc, char const* argv[])
{
    // !(12 + 24) - (4 * 45) / -12;
    // !(3.5 and 4 or true )

    //             var a = 3 < 5;
    // var b = a and (5 == 4);
    // [
    //     var c = 3.0;
    // ]

    // var a: bool;
    // a = false;

    // fun hello(x: int, y: float, z: bool): bool [
    //     var a = x + y;
    //     if (z) [
    //         var b: float = y;
    //     ]

    //     return z;
    // ]

    // fun hai(): int [
    //     if (1 == 2) [
    //         var d = 3;
    //     ]

    //     return hai();
    // ]

    // var x: bool = hello(1, 2.0, a);

    if (argc < 2) {
        std::println("No file provided");
        std::exit(1);
    }

    std::string file { argv[1] };
    jl::Lexer lexer(file);

    std::string file_name = "examples/EList.jun";

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

    jl::CodeGenerator codegen(file_name);
    const auto chunk_map = codegen.generate(stmts);

    if (jl::ErrorHandler::has_error()) {
        return 1;
    }

    codegen.disassemble();

    std::println("_--------------------_____________-----______------_______-----_____--------");

    // const auto [irs, lines] = jl::flatten(chunk_map);
    // jl::disassemble(std::cout, irs, lines);

    const auto chunk = codegen.get_root_chunk();
    // std::println("{}", chunk.disassemble());

    jl::VM vm;
    const auto [res, vars] = vm.run(chunk, chunk_map);

    for (const auto& [name, idx] : chunk.get_variable_map()) {
        std::println("{}\t{}", name, jl::to_string(vars[idx]));
    }

    return 0;
}

/*
        var a = {3, 5, 1, 2};

        for (var item: a) [
            if (item ) [
                printEAKING";
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