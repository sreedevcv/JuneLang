#include "ASTPrinter.hpp"
#include "ErrorHandler.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "backend/IRGen.hpp"
#include "backend/SemanticAnalysis.hpp"
#include "codegen/x86CodeGen.hpp"

#include <cassert>
#include <iostream>
#include <ostream>
#include <print>
#include <string>
#include <utility>

int main(int argc, char const* argv[])
{

    // jl::ArgParser args_parser(argc, argv);
    // const auto params = args_parser.parse();

    // if (!params) {
    //     return 0;
    // }

    // std::string file_name { argv[1] };
    std::string file_name { "../examples/test.june" };
    jl::Lexer lexer(file_name);

    lexer.scan();

    if (jl::ErrorHandler::has_error()) {
        return 1;
    }

    auto tokens = lexer.get_tokens();
    jl::Parser parser(tokens, file_name);
    auto stmts = parser.parseStatements();

    // auto exprs = parser.parse();

    if (jl::ErrorHandler::has_error()) {
        return 1;
    }

    jl::ASTPrinter printer;
    std::cout << printer.print(stmts).str() << std::endl;

    jl::SemanticAnalyzer sm(file_name);
    // std::println("Type check FINAL result: {}", );

    if (sm.type_check(stmts)) {
        std::println("{}", printer.print(stmts).str());

        jl::IRGen cg;
        auto block = cg.generate(stmts);
        block.stream(std::cout);

        auto ir_data = block.get_func_irs();

        for (const auto& data : ir_data) {
            std::println("Fun {}", data.first);
            for (const auto& [idx, data] : data.second.get()->var_data.get_offset_map()) {
                const auto [size, offset] = data;
                std::println("{} - size: {} offset: {}", idx, size, offset);
            }
        }

        jl::x86CodeGen codegen(std::move(ir_data));
        const auto ss = codegen.generate();

        std::println("x86");
        std::println("{}", ss.str());

    } else {
        std::println("Type check Failed");
    }

    // jl::CodeGen code_gen;
    // code_gen.generate(expr.get());

    return 0;
}

// #include "ArgParser.hpp"
// #include "CodeGenerator.hpp"
// #include "ErrorHandler.hpp"
// #include "IROptimizer.hpp"
// #include "Lexer.hpp"
// #include "Parser.hpp"
// #include "StaticAddressPass.hpp"
// #include "VM.hpp"

// #include <cassert>
// #include <cstdint>
// #include <iostream>
// #include <print>
// #include <string>

// int main(int argc, char const* argv[])
// {

//     jl::ArgParser args_parser(argc, argv);
//     const auto params = args_parser.parse();

//     if (!params) {
//         return 0;
//     }

//     std::string file_name { argv[1] };
//     jl::Lexer lexer(file_name);

//     lexer.scan();

//     if (jl::ErrorHandler::has_error()) {
//         return 1;
//     }

//     auto tokens = lexer.get_tokens();
//     jl::Parser parser(tokens, file_name);
//     auto stmts = parser.parseStatements();

//     if (jl::ErrorHandler::has_error()) {
//         return 1;
//     }

//     jl::CodeGenerator codegen(file_name);
//     const auto& [chunk_map, data_section] = codegen.generate(stmts);

//     jl::IROptimizer optmizer(chunk_map);

//     auto optmized = optmizer.optimize();

//     std::println("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~OPT~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
//     for (const auto& [name, chunk] : optmized) {
//         std::println("{}", chunk.disassemble());
//     }
//     std::println("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~OPT~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

//     if (jl::ErrorHandler::has_error()) {
//         return 1;
//     }

//     if (params->debug) {
//         std::println("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~DISASSEMBLY~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
//         codegen.disassemble();
//         std::println("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~PROGRAM-OUTPUT~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
//     }

//     jl::patch_memmory_address(optmized, (uint64_t)data_section.data());
//     auto chunk = codegen.get_root_chunk();

//     jl::VM vm(optmized, (jl::ptr_type)data_section.data());
//     const auto [res, vars] = params->step_by_step
//         ? vm.interactive_execute()
//         : vm.run();

//     if (params->debug || params->step_by_step) {
//         std::println("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~LOCALS/DATA~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

//         for (const auto& [name, temp] : chunk.get_variable_map()) {
//             const auto type = chunk.get_nested_type(jl::TempVar { temp });
//             std::println("{}\t{}", name, jl::VM::pretty_print(vars[temp], type));
//         }

//         data_section.disassemble(std::cout);
//     }

//     // jl::patch_memmory_address(chunk_map, (uint64_t)data_section.data());
//     // auto chunk = codegen.get_root_chunk();

//     // jl::VM vm(chunk_map, (jl::ptr_type)data_section.data());
//     // const auto [res, vars] = params->step_by_step
//     //     ? vm.interactive_execute()
//     //     : vm.run();

//     // if (params->debug || params->step_by_step) {
//     //     std::println("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~LOCALS/DATA~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

//     //     for (const auto& [name, temp] : chunk.get_variable_map()) {
//     //         const auto type = chunk.get_nested_type(jl::TempVar { temp });
//     //         std::println("{}\t{}", name, jl::VM::pretty_print(vars[temp], type));
//     //     }

//     //     data_section.disassemble(std::cout);
//     // }

//     return 0;
// }
