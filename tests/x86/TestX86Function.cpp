
#include "ErrorHandler.hpp"
#include "IRGen_v2.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "Runner.hpp"
#include "SemanticAnalysis.hpp"
#include "codegen/x86/Generator.hpp"
#include "codegen/x86/Passes.hpp"
#include "opt/Optimizer.hpp"
#include "types/TypeContext.hpp"
#include <catch2/catch_test_macros.hpp>
#include <initializer_list>
#include <iostream>
#include <string_view>
#include <utility>

static int run_and_get_exit_code(std::string_view function_name, std::string_view source)
{
    // std::println("aaaaaaa{}aaaaaaa", source);
    auto status = jl::x86::run(source, function_name, "/tmp");
    // std::println("xxxxxx{}xxxxxxxxxx", status.error());
    REQUIRE(status.has_value());
    return status.value();
}

struct CompilationResult {
    jl::x86::pass::AssemblyProgram assembly;
};

static void compile(jl::Module& module, jl::x86::pass::AssemblyProgram& program, std::string_view function)
{
    auto test = module.get_function(function);
    jl::opt::mem2reg(test);
    jl::opt::sccp(test);
    jl::opt::remove_phi_nodes(test);
    std::cout << *test;

    auto x86func = jl::x86::Generator(test).generate();
    auto intervals = jl::x86::pass::liveness_analysis(&x86func);
    auto allocation_map = jl::x86::pass::linear_scan_reg_allocation(&x86func, intervals);
    std::cout << x86func.to_str();
    jl::x86::pass::assign_register(&x86func, allocation_map);
    jl::x86::pass::to_nasm_assembly(program, &x86func);
}

static CompilationResult compile_to_assembly(std::string_view source, std::initializer_list<std::string_view> functions)
{
    std::string file_name = "test.june";

    jl::Lexer lexer(source.data());
    lexer.scan();

    REQUIRE(!jl::ErrorHandler::has_error());

    auto tokens = lexer.get_tokens();

    jl::Parser parser(tokens, file_name);
    auto stmts = parser.parseStatements();

    REQUIRE(!jl::ErrorHandler::has_error());

    jl::TypeContext type_context;
    jl::SemanticAnalyzer sm(file_name, type_context);

    REQUIRE(sm.type_check(stmts));

    jl::IRGenv2 cg(type_context);
    auto module = cg.generate(stmts);

    jl::x86::pass::AssemblyProgram program;
    for (const auto& name : functions) {
        compile(module, program, name);
    }

    if (std::count(program.data_section.cbegin(), program.data_section.cend(), '\n') <= 2) {
        program.data_section = "";
    }

    return {
        .assembly = std::move(program),
    };
}

TEST_CASE("Recursive Sum", "x86 function calling")
{
    const char* source = R"(
fun recursive_sum(n: int): int [
    if (n == 1) return n;
    return n + recursive_sum(n - 1);
]
    )";
    auto result = compile_to_assembly(source, { "recursive_sum" });
    auto exe_assembly = std::format(R"(
global _start

section .text

{}

_start:
    mov rdi, 10
    call recursive_sum
    mov rdi, rax 
    mov rax, 0x3c
    syscall
    ret
    )",
        result.assembly.text_section);

    auto status = run_and_get_exit_code("recursive_sum", exe_assembly);
    REQUIRE(status == 55);
}

TEST_CASE("Factorial", "x86 function calling")
{
    const char* source = R"(
fun multiply(a: int, b: int): int [
    var result = 0;
    while (b > 0) [
        result += a;
        b -= 1;
    ]
    return result;
]

fun factorial(n: int): int [
    if (n == 1) return n;
    return multiply(n, factorial(n - 1));
]
    )";
    auto result = compile_to_assembly(source, { "factorial", "multiply" });
    auto exe_assembly = std::format(R"(
global _start

section .text

{}

_start:
    mov rdi, 5
    call factorial
    mov rdi, rax 
    mov rax, 0x3c
    syscall
    ret
    )",
        result.assembly.text_section);

    auto status = run_and_get_exit_code("factorial", exe_assembly);
    REQUIRE(status == 120);
}
