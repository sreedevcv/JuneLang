#include "CodeGenerator.hpp"
#include "ErrorHandler.hpp"
#include "Interpreter.hpp"
#include "Lexer.hpp"
#include "Operand.hpp"
#include "Parser.hpp"
#include "Resolver.hpp"
#include "VM.hpp"

#include <cassert>
#include <print>

int main(int argc, char const* argv[])
{
    // !(12 + 24) - (4 * 45) / -12;
    // !(3.5 and 4 or true )

    //             var a = 3 < 5;
    // var b = a and (5 == 4);
    // [
    //     var c = 3.0;
    // ]

    jl::Lexer lexer(
        R"( 
            var sum = 0;
            // while (i <= 10)
            for (var i = 0; i <= 10; i += 1)
            [
                sum += i;
            ]

            var b = i;
        )");

    std::string file_name = "examples/EList.jun";

    lexer.scan();

    if (jl::ErrorHandler::has_error()) {
        return 1;
    }

    auto tokens = lexer.get_tokens();
    jl::Parser parser(tokens, file_name);
    auto stmts = parser.parseStatements();

    jl::Interpreter interpreter(file_name);
    jl::Resolver resolver(interpreter, file_name);
    resolver.resolve(stmts);

    if (jl::ErrorHandler::has_error()) {
        return 1;
    }

    jl::CodeGenerator codegen(file_name);
    codegen.generate(stmts);

    if (jl::ErrorHandler::has_error()) {
        return 1;
    }

    codegen.disassemble();
    const auto chunk = codegen.get_chunk();
    jl::VM vm;
    const auto [res, vars] = vm.run(chunk);

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