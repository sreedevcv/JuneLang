#include "ArgParser.hpp"
#include "CodeGenerator.hpp"
#include "ErrorHandler.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "StaticAddressPass.hpp"
#include "VM.hpp"

#include <cassert>
#include <cstdint>
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

    jl::patch_memmory_address(chunk_map, (uint64_t)data_section.data());
    auto chunk = codegen.get_root_chunk();

    jl::VM vm(chunk_map, (jl::ptr_type)data_section.data());
    const auto [res, vars] = params->step_by_step
        ? vm.interactive_execute()
        : vm.run();

    if (params->debug || params->step_by_step) {
        std::println("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~LOCALS/DATA~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

        for (const auto& [name, temp] : chunk.get_variable_map()) {
            const auto type = chunk.get_nested_type(jl::TempVar { temp });
            std::println("{}\t{}", name, jl::VM::pretty_print(vars[temp], type));
        }

        data_section.disassemble(std::cout);
    }

    return 0;
}