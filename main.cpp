#include "ASTPrinter.hpp"
#include "ErrorHandler.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "backend/IRGen_v2.hpp"
// #include "codegen/x86CodeGen.hpp"
#include "frontend/SemanticAnalysis.hpp"
#include "llvm_backend/JuneModule.hpp"
#include "llvm_backend/LLVMIRGen.hpp"
#include "opt/Optimizer.hpp"

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
        // jl::ASTPrinter printer;
        // std::println("{}", printer.print(stmts).str());

        jl::IRGenv2 cg(type_context);
        auto module = cg.generate(stmts);

        auto test = module.get_function("fib");
        std::cout << *test;


        // auto e = test->entry_block();
        // auto ir = test->irs()[2].get();
        // std::cout << "-----------------" << ir->to_str() << "\n";

        // e->remove_ir(ir);

        // std::cout << *test;

        jl::opt::mem2reg(test);
        std::cout << "------------------------------------------\n";
        std::cout << *test;

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
