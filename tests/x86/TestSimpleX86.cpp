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
    const auto [lattice_values, exec_map] = jl::opt::sccp(test);
    jl::opt::dce(test, lattice_values, exec_map);
    jl::opt::remove_phi_nodes(test);
    std::cout << *test;

    auto x86func = jl::x86::Generator(test).generate();
    auto intervals = jl::x86::pass::liveness_analysis(&x86func);
    auto allocation_map = jl::x86::pass::linear_scan_reg_allocation(&x86func, intervals, 6, 6);
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

TEST_CASE("Integer subtraction", "SimpleX86_64")
{
    const char* source = R"(
fun sub_test(a: int, b: int): int [
    return a - b;
]
)";
    auto result = compile_to_assembly(source, "sub_test");
    auto exe_assembly = std::format(R"(
global _start

section .text
{}

_start:
mov rdi, 25
mov rsi, 7
call sub_test
mov rdi, rax
mov rax, 0x3c
syscall
ret
)",
        result.assembly.text_section);

    auto status = run_and_get_exit_code("sub_test", exe_assembly);
    REQUIRE(status == 18);
}

TEST_CASE("Signed integer division", "SimpleX86_64")
{
    const char* source = R"(
fun div_test(a: int, b: int): int [
    return a / b;
]
)";
    auto result = compile_to_assembly(source, "div_test");
    auto exe_assembly = std::format(R"(
global _start

section .text
{}

_start:
mov rdi, 56
mov rsi, -8
call div_test
mov rdi, rax
mov rax, 0x3c
syscall
ret
)",
        result.assembly.text_section);

    auto status = run_and_get_exit_code("div_test", exe_assembly);
    REQUIRE(status == static_cast<unsigned char>(-7));
}

TEST_CASE("Logical and and or", "SimpleX86_64")
{
    const char* source = R"(
fun logical_test(a: int, b: int): int [
    if (a > 0 and b > 0) return 1;
    if (a < 0 or b < 0) return 2;
    return 3;
]
)";
    auto result = compile_to_assembly(source, "logical_test");
    auto exe_assembly = std::format(R"(
global _start

section .text
{}

_start:
mov rdi, 5
mov rsi, 3
call logical_test
mov rdi, rax
mov rax, 0x3c
syscall
ret
)",
        result.assembly.text_section);

    auto status = run_and_get_exit_code("logical_test", exe_assembly);
    REQUIRE(status == 1);
}

TEST_CASE("Bitwise and, or, xor", "SimpleX86_64")
{
    const char* source = R"(
fun bitwise_test(a: int, b: int): int [
    var c = a & b;
    var d = a ^ b;
    return c | d;
]
)";
    auto result = compile_to_assembly(source, "bitwise_test");
    auto exe_assembly = std::format(R"(
global _start

section .text
{}

_start:
mov rdi, 12
mov rsi, 10
call bitwise_test
mov rdi, rax
mov rax, 0x3c
syscall
ret
)",
        result.assembly.text_section);

    auto status = run_and_get_exit_code("bitwise_test", exe_assembly);
    REQUIRE(status == 14);
}

TEST_CASE("Nested loops", "SimpleX86_64")
{
    const char* source = R"(
fun nested_loop(n: int): int [
    var sum = 0;
    for (var i = 0; i < n; i += 1) [
        for (var j = 0; j < n; j += 1) [
            sum += 1;
        ]
    ]
    return sum;
]
)";
    auto result = compile_to_assembly(source, "nested_loop");
    auto exe_assembly = std::format(R"(
global _start

section .text
{}

_start:
mov rdi, 4
call nested_loop
mov rdi, rax
mov rax, 0x3c
syscall
ret
)",
        result.assembly.text_section);

    auto status = run_and_get_exit_code("nested_loop", exe_assembly);
    REQUIRE(status == 16);
}

TEST_CASE("Integer comparisons", "SimpleX86_64")
{
    const char* source = R"(
fun compare_test(a: int, b: int): int [
    if (a == b) return 1;
    if (a != b and a < b) return 2;
    if (a > b) return 3;
    return 4;
]
)";
    auto result = compile_to_assembly(source, "compare_test");
    auto exe_assembly = std::format(R"(
global _start

section .text
{}

_start:
mov rdi, 8
mov rsi, 5
call compare_test
mov rdi, rax
mov rax, 0x3c
syscall
ret
)",
        result.assembly.text_section);

    auto status = run_and_get_exit_code("compare_test", exe_assembly);
    REQUIRE(status == 3);
}

TEST_CASE("Six integer arguments", "SimpleX86_64")
{
    const char* source = R"(
fun sum_six(a: int, b: int, c: int, d: int, e: int, f: int): int [
    return a + b + c + d + e + f;
]
)";
    auto result = compile_to_assembly(source, "sum_six");
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

TEST_CASE("If else if chain", "SimpleX86_64")
{
    const char* source = R"(
fun classify(x: int): int [
    if (x == 0) return 0;
    else if (x < 0) return 1;
    else return 2;
]
)";
    auto result = compile_to_assembly(source, "classify");
    auto exe_assembly = std::format(R"(
global _start

section .text
{}

_start:
mov rdi, -7
call classify
mov rdi, rax
mov rax, 0x3c
syscall
ret
)",
        result.assembly.text_section);

    auto status = run_and_get_exit_code("classify", exe_assembly);
    REQUIRE(status == 1);
}

TEST_CASE("Early return from loop", "SimpleX86_64")
{
    const char* source = R"(
fun early_return(n: int): int [
    var i = 0;
    while (i < n) [
        if (i == 5) return i;
        i += 1;
    ]
    return i;
]
)";
    auto result = compile_to_assembly(source, "early_return");
    auto exe_assembly = std::format(R"(
global _start

section .text
{}

_start:
mov rdi, 100
call early_return
mov rdi, rax
mov rax, 0x3c
syscall
ret
)",
        result.assembly.text_section);

    auto status = run_and_get_exit_code("early_return", exe_assembly);
    REQUIRE(status == 5);
}

TEST_CASE("Float arithmetic", "SimpleX86_64")
{
    const char* source = R"(
fun float_math(f: float): float [
    var a = f * 2.0;
    var b = a - 4.0;
    return b / 2.0;
]
)";
    auto result = compile_to_assembly(source, "float_math");
    auto exe_assembly = std::format(R"(
global _start

section .data
{}
float_math_input dq 5.0

section .text
{}

_start:
movsd xmm0, [float_math_input]
call float_math
movq rdi, xmm0
mov rax, 0x3c
syscall
ret
)",
        result.assembly.data_section,
        result.assembly.text_section);

    auto status = run_and_get_exit_code("float_math", exe_assembly);
    REQUIRE(status == (std::bit_cast<int64_t>(3.0) & 0xFF));
}

TEST_CASE("Three float arguments", "SimpleX86_64")
{
    const char* source = R"(
fun sum_three_floats(a: float, b: float, c: float): float [
    return a + b + c;
]
)";
    auto result = compile_to_assembly(source, "sum_three_floats");
    auto exe_assembly = std::format(R"(
global _start

section .data
{}
sum_three_floats_a dq 1.0
sum_three_floats_b dq 2.0
sum_three_floats_c dq 3.0

section .text
{}

_start:
movsd xmm0, [sum_three_floats_a]
movsd xmm1, [sum_three_floats_b]
movsd xmm2, [sum_three_floats_c]
call sum_three_floats
movq rdi, xmm0
mov rax, 0x3c
syscall
ret
)",
        result.assembly.data_section,
        result.assembly.text_section);

    auto status = run_and_get_exit_code("sum_three_floats", exe_assembly);
    REQUIRE(status == (std::bit_cast<int64_t>(6.0) & 0xFF));
}

TEST_CASE("Float comparison", "SimpleX86_64")
{
    const char* source = R"(
fun float_compare(a: float, b: float): int [
    if (a < b) return 1;
    if (a > b) return 2;
    return 3;
]
)";
    auto result = compile_to_assembly(source, "float_compare");
    auto exe_assembly = std::format(R"(
global _start

section .data
{}
float_compare_a dq 2.0
float_compare_b dq 7.0

section .text
{}

_start:
movsd xmm0, [float_compare_a]
movsd xmm1, [float_compare_b]
call float_compare
mov rdi, rax
mov rax, 0x3c
syscall
ret
)",
        result.assembly.data_section,
        result.assembly.text_section);

    auto status = run_and_get_exit_code("float_compare", exe_assembly);
    REQUIRE(status == 1);
}

TEST_CASE("Boolean argument", "SimpleX86_64")
{
    const char* source = R"(
fun bool_test(b: bool): int [
    if (b) return 42;
    return 0;
]
)";
    auto result = compile_to_assembly(source, "bool_test");
    auto exe_assembly = std::format(R"(
global _start

section .text
{}

_start:
mov rdi, 1
call bool_test
mov rdi, rax
mov rax, 0x3c
syscall
ret
)",
        result.assembly.text_section);

    auto status = run_and_get_exit_code("bool_test", exe_assembly);
    REQUIRE(status == 42);
}

TEST_CASE("Mixed control flow", "SimpleX86_64")
{
    const char* source = R"(
fun mixed(n: int): int [
    var sum = 0;
    for (var i = 0; i < n; i += 1) [
        if (i < 5) [
            sum += i;
        ] else [
            sum -= 1;
        ]
    ]
    return sum;
]
)";
    auto result = compile_to_assembly(source, "mixed");
    auto exe_assembly = std::format(R"(
global _start

section .text
{}

_start:
mov rdi, 8
call mixed
mov rdi, rax
mov rax, 0x3c
syscall
ret
)",
        result.assembly.text_section);

    auto status = run_and_get_exit_code("mixed", exe_assembly);
    REQUIRE(status == 7);
}
