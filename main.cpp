#include "ASTPrinter.hpp"
#include "ErrorHandler.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "backend/IRGen.hpp"
#include "codegen/x86CodeGen.hpp"
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

    jl::ASTPrinter printer;
    std::cout << printer.print(stmts).str() << std::endl;

    jl::SemanticAnalyzer sm(file_name);
    // std::println("Type check FINAL result: {}", );
    //

    if (sm.type_check(stmts)) {
        // jl::Module module(file_name);
        // module.module().print(llvm::outs(), nullptr);

        jl::LLVMIRGen ir_gen(file_name);
        // ir_gen.emit(stmts).module().print(llvm::outs(), nullptr);

        std::error_code ec;
        llvm::raw_fd_ostream file("test.ll", ec);
        ir_gen.emit(stmts).module().print(file, nullptr);

        file.close();

    } else {
        std::println("Type  check failed");
    }

    return 0;

    if (sm.type_check(stmts)) {
        std::println("{}", printer.print(stmts).str());

        jl::IRGen cg;
        auto block = cg.generate(stmts);
        block.stream(std::cout);

        auto ir_data = block.get_func_irs();

        for (const auto& data : ir_data) {
            std::println("Fun {}", data.first);
            for (const auto& [idx, data] : data.second.get()->var_data.get_offset_map()) {
                const auto& [size, offset, type] = data;
                std::println("{} - size: {} offset: {} type: {}", idx, size, offset, type ? type.get()->to_str() : "data");
            }
        }

        jl::x86CodeGen codegen(std::move(ir_data));
        const auto ss = codegen.generate();

        std::println("{}", ss.str());

        std::ofstream file("test.asm");
        file << ss.str();
        file.close();

    } else {
        std::println("Type check Failed");
    }

    // jl::CodeGen code_gen;
    // code_gen.generate(expr.get());

    return 0;
}
