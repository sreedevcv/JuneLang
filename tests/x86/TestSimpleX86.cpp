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

#include <bit>
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <string_view>
#include <utility>

int run_and_get_exit_code(std::string_view function_name, std::string_view source)
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

CompilationResult compile_to_assembly(std::string_view source, std::string_view function_name)
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

    auto test = module.get_function(function_name);

    jl::opt::mem2reg(test);
    jl::opt::sccp(test);
    jl::opt::remove_phi_nodes(test);
    std::cout << *test;

    auto x86func = jl::x86::Generator(test).generate();
    auto intervals = jl::x86::pass::liveness_analysis(&x86func);
    auto allocation_map = jl::x86::pass::linear_scan_reg_allocation(&x86func, intervals);
    std::cout << x86func.to_str();
    jl::x86::pass::assign_register(&x86func, allocation_map);
    jl::x86::pass::AssemblyProgram program;
    jl::x86::pass::to_nasm_assembly(program, &x86func);
    if (std::count(program.data_section.cbegin(), program.data_section.cend(), '\n') <= 2) {
        program.data_section = "";
    }

    return {
        .assembly = std::move(program),
    };
}

TEST_CASE("Fibonacci Number", "SimpleX86_64")
{
    const char* source = R"(
fun fib(n: int): int [
    var a = 0;
    var b = 1;
    var c = 0;
    var i = 1;

    if (n == 0) return 0;

    while (i < n) [
        c = b;
        b = a + b;
        a = c;
        i = i + 1;
    ]

    return b;
]
    )";
    auto result = compile_to_assembly(source, "fib");
    auto exe_assembly = std::format(R"(
global _start

section .text

{}

_start:
    mov rdi, 9
    call fib
    mov rdi, rax 
    mov rax, 0x3c
    syscall
    ret
    )",
        result.assembly.text_section);

    // std::println("----{}----", exe_assembly);

    auto status = run_and_get_exit_code("fib", exe_assembly);
    REQUIRE(status == 34);
}

TEST_CASE("Simple float test", "SimpleX86_64")
{
    const char* source = R"(
fun float_test(f1: float): float [
    var a = f1 + 2.5;
    a += 3.4;
    a += 1.0;
    if (a > 10.0) [
        return 2.0;
    ]
    return a;
]
)";
    auto result = compile_to_assembly(source, "float_test");
    auto exe_assembly = std::format(R"(
global _start

section .data
{}
test_input dq 2.5

section .text
{}


_start:
movsd xmm0, [test_input] 
call float_test
movq rdi, xmm0
mov rax, 0x3c
syscall
ret
)",
        result.assembly.data_section,
        result.assembly.text_section);

    auto status = run_and_get_exit_code("float_test", exe_assembly);
    REQUIRE(status == 205);
}

TEST_CASE("Sum of first N natural numbers", "SimpleX86_64")
{
    const char* source = R"(
fun sum(n: int): int [
    var sum = 0;

    for (var i = 0; i <= n; i += 1) [
        sum += i;
    ]

    return sum;
]
)";
    auto result = compile_to_assembly(source, "sum");
    auto exe_assembly = std::format(R"(
global _start

section .text
{}


_start:
mov rdi, 10
call sum
mov rdi, rax
mov rax, 0x3c
syscall
ret
)",
        result.assembly.text_section);

    auto status = run_and_get_exit_code("sum", exe_assembly);
    REQUIRE(status == 55);
}

TEST_CASE("Floating point sum", "SimpleX86_64")
{
    const char* source = R"(
fun fsum(n: float): float [
    var sum = 1.0;

    for (var i = 0.0; i < n; i += 1.0) [
        sum += 0.5; 
    ]

    return sum;
]
)";
    auto result = compile_to_assembly(source, "fsum");
    auto exe_assembly = std::format(R"(
global _start

section .data
{}
test_input dq 10.0

section .text
{}


_start:
movsd xmm0, [test_input] 
call fsum
movq rdi, xmm0
mov rax, 0x3c
syscall
ret
)",
        result.assembly.data_section,
        result.assembly.text_section);

    auto status = run_and_get_exit_code("fsum", exe_assembly);
    REQUIRE(status == (std::bit_cast<int64_t>(5.0) & 0xFF));
}

TEST_CASE("factorial with signed multiplication", "SimpleX86_64")
{
    const char* source = R"(
fun imul_fact(n: int): int [
    var sum = 1;
    for (var i = 2; i <= n; i += 1) [
        sum *= i;
    ]
    return sum;
]
)";
    auto result = compile_to_assembly(source, "imul_fact");
    auto exe_assembly = std::format(R"(
global _start

section .text
{}


_start:
mov rdi, 5
call imul_fact
mov rdi, rax
mov rax, 0x3c
syscall
ret
)",
        result.assembly.text_section);

    auto status = run_and_get_exit_code("imul_fact", exe_assembly);
    REQUIRE(status == 120);
}

TEST_CASE("factorial with fp multiplication", "SimpleX86_64")
{
    const char* source = R"(
fun mulsd_fact(n: float): float [
    var sum = 1.0;
    for (var i = 2.0; i <= n; i += 1.0) [
        sum *= i;
    ]
    return sum;
]
)";
    auto result = compile_to_assembly(source, "mulsd_fact");
    auto exe_assembly = std::format(R"(
global _start

section .data
{}
test_input dq 5.0

section .text
{}


_start:
movsd xmm0, [test_input] 
call mulsd_fact
movq rdi, xmm0
mov rax, 0x3c
syscall
ret
)",
        result.assembly.data_section,
        result.assembly.text_section);

    auto status = run_and_get_exit_code("mulsd_fact", exe_assembly);
    REQUIRE(status == (std::bit_cast<int64_t>(120.0) & 0xFF));
}
