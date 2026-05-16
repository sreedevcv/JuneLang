#include "ASTPrinter.hpp"
#include "ErrorHandler.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "backend/IRGen_v2.hpp"
// #include "codegen/x86CodeGen.hpp"
#include "frontend/SemanticAnalysis.hpp"
#include "llvm_backend/JuneModule.hpp"
#include "llvm_backend/LLVMIRGen.hpp"

#include <cassert>
#include <fstream>
#include <iostream>
#include <llvm/Support/raw_ostream.h>
#include <ostream>
#include <print>
#include <string>
#include <system_error>
#include <utility>

#define CUSTOM_BACKEND

int main(int argc, char const* argv[])
{
    std::string file_name = argc <= 1 ? "../examples/test.june" : argv[1];
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

    jl::TypeContext type_context;
    jl::SemanticAnalyzer sm(file_name, type_context);

    if (sm.type_check(stmts)) {
#ifdef CUSTOM_BACKEND
        jl::ASTPrinter printer;
        std::println("{}", printer.print(stmts).str());

        jl::IRGenv2 cg(type_context);
        auto module = cg.generate(stmts);
        // module.stream(std::cout);

        std::println("Functions: ");
        std::cout << module;

        // auto ir_data = module.basic_blocks();

        // for (const auto& data : ir_data) {
        //     std::println("Fun {}", data.first);
        //     for (const auto& [idx, data] :
        //         data.second->var_data.get_offset_map()) {
        //         const auto& [size, offset, type] = data;
        //         std::println("{} - size: {} offset: {} type: {}", idx, size, offset,
        //             type ? type->to_str() : "data");
        //     }
        // }

        // jl::x86CodeGen codegen(std::move(ir_data));
        // const auto ss = codegen.generate();

        // std::println("{}", ss.str());

        // std::ofstream file("test.asm");
        // file << ss.str();
        // file.close();

#else
        // jl::Module module(file_name);
        // module.module().print(llvm::outs(), nullptr);
        jl::LLVMIRGen ir_gen(file_name, type_context);
        // ir_gen.emit(stmts).module().print(llvm::outs(), nullptr);

        std::error_code ec;
        llvm::raw_fd_ostream file("test.ll", ec);
        ir_gen.emit(stmts).module().print(file, nullptr);

        file.close();
#endif

    } else {
        std::println("Type  check failed");
    }

    return 0;
}
