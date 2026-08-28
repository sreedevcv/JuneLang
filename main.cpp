#include "ErrorHandler.hpp"
#include "Function.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "backend/IRGen_v2.hpp"
#include "backend/codegen/x86/Generator.hpp"
#include "codegen/x86/Passes.hpp"
#include "frontend/SemanticAnalysis.hpp"
#include "opt/Optimizer.hpp"

#include <cassert>
#include <iostream>
#include <print>
#include <string>

#define CUSTOM_BACKEND

void compile_function(jl::Function* function)
{
    jl::opt::mem2reg(function);
    jl::opt::sccp(function);
    jl::opt::remove_phi_nodes(function);

    jl::x86::Generator x86gen(function);
    auto x86func = x86gen.generate();
    std::cout << *function;

    std::println("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~X86_64~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    std::println("{}", x86func.to_str());
    // auto intervals = jl::x86::pass::liveness_analysis(&x86func);
    // jl::x86::pass::linear_scan_reg_allocation(&x86func, intervals);
}

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
        //   compile_function(function.get());
        //}

        auto func = module.get_function("fib");

        // std::cout << *func;
        // std::println("----------------------------------------------------------------");
        jl::opt::mem2reg(func);
        // std::cout << *func;
        // std::println("----------------------------------------------------------------");
        jl::opt::sccp(func);
        // std::cout << *func;
        // std::println("----------------------------------------------------------------");
        jl::opt::remove_phi_nodes(func);
        std::cout << *func;
        std::println("----------------------------------------------------------------");

        jl::x86::Generator x86gen(func);
        auto x86func = x86gen.generate();
        std::println("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~X86_64~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
        std::println("{}", x86func.to_str());

        auto intervals = jl::x86::pass::liveness_analysis(&x86func);
        auto allocation_map = jl::x86::pass::linear_scan_reg_allocation(&x86func, intervals);
        jl::x86::pass::assign_register(&x86func, allocation_map);

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
