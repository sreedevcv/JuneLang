#include "CodeGenerator.hpp"
#include "ErrorHandler.hpp"
#include "Interpreter.hpp"
#include "Lexer.hpp"
#include "Operand.hpp"
#include "Parser.hpp"
#include "Resolver.hpp"
#include "VM.hpp"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <print>
#include <string>

int main(int argc, char const* argv[])
{
    bool is_interactive = false;

    if (argc < 2) {
        std::println("No file provided");
        std::exit(1);
    }

    if (argc >= 3 && std::strcmp(argv[2], "-i") == 0) {
        is_interactive = true;
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
    const auto& [chunk_map, data_section] = codegen.generate(stmts);

    if (jl::ErrorHandler::has_error()) {
        return 1;
    }

    codegen.disassemble();

    std::println("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

    const auto chunk = codegen.get_root_chunk();

    jl::VM vm;
    const auto [res, vars] = is_interactive
        ? vm.interactive_execute(chunk, chunk_map, data_section)
        : vm.run(chunk, chunk_map, data_section);

    for (const auto& [name, temp] : chunk.get_variable_map()) {
        std::println("{}\t{}", name, jl::to_string(vars[temp]));
    }

    return 0;
}