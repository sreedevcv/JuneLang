
#include "ErrorHandler.hpp"
#include "IRGen.hpp"
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
    const auto [lattice_values, exec_map] = jl::opt::sccp(test);
    jl::opt::dce(test, lattice_values, exec_map);
    jl::opt::remove_phi_nodes(test);
    std::cout << *test;

    auto x86func = jl::x86::Generator(test).generate();
    auto intervals = jl::x86::pass::liveness_analysis(&x86func);
    auto allocation_map = jl::x86::pass::linear_scan_reg_allocation(&x86func, intervals, 12, 12);
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

TEST_CASE("Fibonacci", "x86 function calling")
{
    const char* source = R"(
fun fib(n: int): int [
    if (n == 0) return 0;
    if (n == 1) return 1;
    return fib(n - 1) + fib(n - 2);
]
    )";
    auto result = compile_to_assembly(source, { "fib" });
    auto exe_assembly = std::format(R"(
global _start

section .text

{}

_start:
    mov rdi, 10
    call fib
    mov rdi, rax 
    mov rax, 0x3c
    syscall
    ret
    )",
        result.assembly.text_section);

    auto status = run_and_get_exit_code("fib", exe_assembly);
    REQUIRE(status == 55);
}

TEST_CASE("McCarthy 91", "x86 function calling")
{
    const char* source = R"(
fun mc91(n: int): int [
    if (n > 100) return n - 10;
    return mc91(mc91(n + 11));
]
    )";
    auto result = compile_to_assembly(source, { "mc91" });
    auto exe_assembly = std::format(R"(
global _start

section .text

{}

_start:
    mov rdi, 95
    call mc91
    mov rdi, rax 
    mov rax, 0x3c
    syscall
    ret
    )",
        result.assembly.text_section);

    auto status = run_and_get_exit_code("mc91", exe_assembly);
    REQUIRE(status == 91);
}

TEST_CASE("GCD with helper", "x86 function calling")
{
    const char* source = R"(
fun slow_mod(a: int, b: int): int [
    while (a >= b) [
        a = a - b;
    ]
    return a;
]

fun gcd(a: int, b: int): int [
    if (b == 0) return a;
    return gcd(b, slow_mod(a, b));
]
    )";
    auto result = compile_to_assembly(source, { "slow_mod", "gcd" });
    auto exe_assembly = std::format(R"(
global _start

section .text

{}

_start:
    mov rdi, 48
    mov rsi, 18
    call gcd
    mov rdi, rax 
    mov rax, 0x3c
    syscall
    ret
    )",
        result.assembly.text_section);

    auto status = run_and_get_exit_code("gcd", exe_assembly);
    REQUIRE(status == 6);
}

TEST_CASE("Power function", "x86 function calling")
{
    const char* source = R"(
fun half(n: int): int [
    return n / 2;
]

fun power(base: int, exp: int): int [
    if (exp == 0) return 1;
    if (exp == 1) return base;
    var p = power(base, half(exp));
    p = p * p;
    if (exp == half(exp) * 2) return p;
    return p * base;
]
    )";
    auto result = compile_to_assembly(source, { "half", "power" });
    auto exe_assembly = std::format(R"(
global _start

section .text

{}

_start:
    mov rdi, 3
    mov rsi, 4
    call power
    mov rdi, rax 
    mov rax, 0x3c
    syscall
    ret
    )",
        result.assembly.text_section);

    auto status = run_and_get_exit_code("power", exe_assembly);
    REQUIRE(status == 81);
}

TEST_CASE("Digit sum", "x86 function calling")
{
    const char* source = R"(
fun last_digit(n: int): int [
    while (n >= 10) [
        n = n - 10;
    ]
    return n;
]

fun digit_sum(n: int): int [
    if (n < 10) return n;
    return last_digit(n) + digit_sum(n / 10);
]
    )";
    auto result = compile_to_assembly(source, { "last_digit", "digit_sum" });
    auto exe_assembly = std::format(R"(
global _start

section .text

{}

_start:
    mov rdi, 12345
    call digit_sum
    mov rdi, rax 
    mov rax, 0x3c
    syscall
    ret
    )",
        result.assembly.text_section);

    auto status = run_and_get_exit_code("digit_sum", exe_assembly);
    REQUIRE(status == 15);
}

TEST_CASE("Range sums", "x86 function calling")
{
    const char* source = R"(
fun sum_range(a: int, b: int): int [
    if (a > b) return 0;
    return a + sum_range(a + 1, b);
]

fun sum_odd_range(a: int, b: int): int [
    if (a > b) return 0;
    if (a == (a / 2) * 2) return sum_odd_range(a + 1, b);
    return a + sum_odd_range(a + 1, b);
]

fun total(a: int, b: int, c: int): int [
    return sum_range(a, b) + sum_odd_range(1, c);
]
    )";
    auto result = compile_to_assembly(source, { "sum_range", "sum_odd_range", "total" });
    auto exe_assembly = std::format(R"(
global _start

section .text

{}

_start:
    mov rdi, 2
    mov rsi, 4
    mov rdx, 3
    call total
    mov rdi, rax 
    mov rax, 0x3c
    syscall
    ret
    )",
        result.assembly.text_section);

    auto status = run_and_get_exit_code("total", exe_assembly);
    REQUIRE(status == 13);
}

TEST_CASE("Chained function calls", "x86 function calling")
{
    const char* source = R"(
fun c(n: int): int [
    if (n == 0) return 1;
    return c(n - 1) + 1;
]

fun b(n: int): int [
    return c(n) * 2;
]

fun a(n: int): int [
    return b(n) + c(n);
]
    )";
    auto result = compile_to_assembly(source, { "c", "b", "a" });
    auto exe_assembly = std::format(R"(
global _start

section .text

{}

_start:
    mov rdi, 3
    call a
    mov rdi, rax 
    mov rax, 0x3c
    syscall
    ret
    )",
        result.assembly.text_section);

    auto status = run_and_get_exit_code("a", exe_assembly);
    REQUIRE(status == 12);
}

TEST_CASE("Two functions with Six integer arguments", "x86 function calling")
{
    const char* source = R"(
fun add_three(a: int, b: int, c: int): int [
    return a + b + c;
]

fun sum_six(a: int, b: int, c: int, d: int, e: int, f: int): int [
    return add_three(a, b, c) + add_three(d, e, f);
]
    )";
    auto result = compile_to_assembly(source, { "add_three", "sum_six" });
    auto exe_assembly = std::format(R"(
global _start

section .text

{}

_start:
    mov rdi, 1
    mov rsi, 2
    mov rdx, 3
    mov rcx, 4
    mov r8, 5
    mov r9, 6
    call sum_six
    mov rdi, rax 
    mov rax, 0x3c
    syscall
    ret
    )",
        result.assembly.text_section);

    auto status = run_and_get_exit_code("sum_six", exe_assembly);
    REQUIRE(status == 21);
}

TEST_CASE("For loop with function call", "x86 function calling")
{
    const char* source = R"(
fun add_one(n: int): int [
    return n + 1;
]

fun sum_incremented(n: int): int [
    var s = 0;
    for (var i = 0; i < n; i += 1) [
        s = s + add_one(i);
    ]
    return s;
]
    )";
    auto result = compile_to_assembly(source, { "add_one", "sum_incremented" });
    auto exe_assembly = std::format(R"(
global _start

section .text

{}

_start:
    mov rdi, 5
    call sum_incremented
    mov rdi, rax 
    mov rax, 0x3c
    syscall
    ret
    )",
        result.assembly.text_section);

    auto status = run_and_get_exit_code("sum_incremented", exe_assembly);
    REQUIRE(status == 15);
}

TEST_CASE("While loop with function call", "x86 function calling")
{
    const char* source = R"(
fun accumulate(a: int, b: int): int [
    return a + b;
]

fun sum_pairs(n: int): int [
    var s = 0;
    var i = 0;
    while (i < n) [
        s = accumulate(s, i);
        i = i + 1;
    ]
    return s;
]
    )";
    auto result = compile_to_assembly(source, { "accumulate", "sum_pairs" });
    auto exe_assembly = std::format(R"(
global _start

section .text

{}

_start:
    mov rdi, 5
    call sum_pairs
    mov rdi, rax 
    mov rax, 0x3c
    syscall
    ret
    )",
        result.assembly.text_section);

    auto status = run_and_get_exit_code("sum_pairs", exe_assembly);
    REQUIRE(status == 10);
}

TEST_CASE("Float arguments", "x86 function calling")
{
    const char* source = R"(
fun scale(a: float, b: float): float [
    return a * b;
]

fun check_scale(a: float, b: float, expected: float): int [
    var result = scale(a, b);
    if (result == expected) return 1;
    return 0;
]
    )";
    auto result = compile_to_assembly(source, { "scale", "check_scale" });
    auto exe_assembly = std::format(R"(
global _start

section .text

{}

_start:
    mov rax, 0x4000000000000000
    movq xmm0, rax
    mov rax, 0x4008000000000000
    movq xmm1, rax
    mov rax, 0x4018000000000000
    movq xmm2, rax
    call check_scale
    mov rdi, rax 
    mov rax, 0x3c
    syscall
    ret
    )",
        result.assembly.text_section);

    auto status = run_and_get_exit_code("check_scale", exe_assembly);
    REQUIRE(status == 1);
}
