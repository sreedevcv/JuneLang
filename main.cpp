#include "ErrorHandler.hpp"
#include "Function.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "backend/IRGen.hpp"
#include "backend/codegen/x86/Generator.hpp"
#include "codegen/x86/Instruction.hpp"
#include "codegen/x86/MachineFunction.hpp"
#include "codegen/x86/Passes.hpp"
#include "frontend/SemanticAnalysis.hpp"
#include "llvm_backend/LLVMIRGen.hpp"
#include "opt/Optimizer.hpp"

#include <cassert>
#include <fstream>
#include <iostream>
#include <print>
#include <string>

#define CUSTOM_BACKEND

jl::x86::MachineFunction compile_function(jl::Module& module, std::string_view name)
{
    auto func = module.get_function(name);

    // std::cout << *func;
    // std::println("------------------------mem2reg-----------------------------------");
    jl::opt::mem2reg(func);
    // std::cout << *func;
    // std::println("--------------------------sccp----------------------------------");
    const auto [lattice_values, exec_map] = jl::opt::sccp(func);
    // std::cout << *func;
    // std::println("--------------------------dce----------------------------------");
    jl::opt::dce(func, lattice_values, exec_map);
    // std::cout << *func;
    // std::println("-----------------------phi-removal-----------------------------------");
    jl::opt::remove_phi_nodes(func);
    std::println("----------------------------final-------------------------------");
    std::cout << *func;

    jl::x86::Generator x86gen(func);
    auto x86func = x86gen.generate();
    std::println("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~X86_64~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    std::println("{}", x86func.to_str());

    auto intervals = jl::x86::pass::liveness_analysis(&x86func);
    auto allocation_result = jl::x86::pass::linear_scan_reg_allocation(&x86func, intervals, 6, 6);
    jl::x86::pass::assign_register(&x86func, allocation_result);
    return std::move(x86func);
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

        jl::x86::pass::AssemblyProgram program;

        // auto f1 = compile_function(module, "sccp_test");
        // jl::x86::pass::to_nasm_assembly(program, &f1);

        for (auto& func : module.functions()) {
            auto f = compile_function(module, func->name());
            jl::x86::pass::to_nasm_assembly(program, &f);
        }

        if (std::count(program.data_section.cbegin(), program.data_section.cend(), '\n') <= 2) {
            program.data_section = "";
        }

        const auto assembly = std::format(R"(section .data 
{}

section .text 

{}
)",
            program.data_section, program.text_section);

        std::print("{}", assembly);

        std::ofstream out("out.asm");
        out << assembly;
        out.close();

        return 0;
#else
        jl::LLVMIRGen ir_gen(file_name, type_context);

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
