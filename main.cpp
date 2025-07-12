#include "ArgParser.hpp"
#include "CodeGenerator.hpp"
#include "ErrorHandler.hpp"
#include "Interpreter.hpp"
#include "Lexer.hpp"
#include "Operand.hpp"
#include "Parser.hpp"
#include "Resolver.hpp"
#include "VM.hpp"

#include <cassert>
#include <iostream>
#include <print>
#include <string>

int main(int argc, char const* argv[])
{

    jl::ArgParser args_parser(argc, argv);
    const auto params = args_parser.parse();

    if (!params) {
        return 0;
    }

    std::string file_name { argv[1] };
    jl::Lexer lexer(file_name);

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
    const auto& [chunk_map, data_section] = codegen.generate(stmts);

    if (jl::ErrorHandler::has_error()) {
        return 1;
    }

    if (params->debug) {
        std::println("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~DISASSEMBLY~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
        codegen.disassemble();
        std::println("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~PROGRAM-OUTPUT~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    }

    auto chunk = codegen.get_root_chunk();

    jl::VM vm;
    const auto [res, vars] = params->step_by_step
        ? vm.interactive_execute(chunk, chunk_map, data_section)
        : vm.run(chunk, chunk_map, data_section);

    if (params->debug || params->step_by_step) {
        std::println("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~LOCALS/DATA~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

        for (const auto& [name, temp] : chunk.get_variable_map()) {
            std::println("{}\t{}", name, jl::to_string(vars[temp]));
        }

        data_section.disassemble(std::cout);
    }

    return 0;
}