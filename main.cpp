#include "ErrorHandler.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "backend/IRGen_v2.hpp"
#include "frontend/SemanticAnalysis.hpp"
#include "ir/Return.hpp"
#include "opt/Optimizer.hpp"

#include <cassert>
#include <iostream>
#include <print>
#include <string>

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

    if (jl::ErrorHandler::has_error()) {
        return 1;
    }

    jl::TypeContext type_context;
    jl::SemanticAnalyzer sm(file_name, type_context);

    if (sm.type_check(stmts)) {
#ifdef CUSTOM_BACKEND
        jl::IRGenv2 cg(type_context);
        auto module = cg.generate(stmts);

        // for (auto& function : module.functions()) {
        //     std::println("before mem2reg ir count: {}", function.get()->irs().size());
        //     jl::opt::mem2reg(function.get());
        //     std::println("after mem2reg ir count: {}", function.get()->irs().size());
        // }

        // std::cout << module;

        auto func = module.get_function("test");

        std::cout << *func;
        std::println("----------------------------------------------------------------");

        jl::opt::mem2reg(func);

        std::cout << *func;
        std::println("----------------------------------------------------------------");

        jl::opt::sccp(func);
        jl::opt::remove_phi_nodes(func);

        std::println("----------------------------------------------------------------");

        std::cout << *func;
        std::cout << func->entry_block()->get_name();

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
