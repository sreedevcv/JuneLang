#include "VM.hpp"
#include "catch2/catch_test_macros.hpp"

#include "CodeGenerator.hpp"
#include "ErrorHandler.hpp"
#include "Interpreter.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "Resolver.hpp"
#include <utility>

static std::pair<
    std::vector<jl::Operand>,
    std::unordered_map<std::string, uint32_t>>
compile(const char* source_code)
{
    using namespace jl;

    std::string file_name = "test.jun";
    jl::Lexer lexer(source_code);
    lexer.scan();

    REQUIRE(jl::ErrorHandler::has_error() == false);

    auto tokens = lexer.get_tokens();
    jl::Parser parser(tokens, file_name);
    auto stmts = parser.parseStatements();

    REQUIRE(jl::ErrorHandler::has_error() == false);

    jl::Interpreter interpreter(file_name);
    jl::Resolver resolver(interpreter, file_name);
    resolver.resolve(stmts);

    REQUIRE(jl::ErrorHandler::has_error() == false);

    jl::CodeGenerator codegen(file_name);
    const auto [chunk_map, data_section] = codegen.generate(stmts);

    REQUIRE(jl::ErrorHandler::has_error() == false);

    jl::VM vm;
    const auto chunk = codegen.get_root_chunk();
    const auto [status, temp_vars] = vm.run(chunk, chunk_map, data_section);
    const auto var_map = chunk.get_variable_map();

    return { temp_vars, var_map };
}

TEST_CASE("Expressions", "[Codegen]")
{
    using namespace jl;

    const auto [temp_vars, var_map] = compile(
        R"( var a = 10 + 2;
        var b = (a * 2) - (3 + 1);
        var c = b / 10.0;
)");

    const auto a_value = temp_vars[var_map.at("a")];
    const auto b_value = temp_vars[var_map.at("b")];
    const auto c_value = temp_vars[var_map.at("c")];

    REQUIRE(std::get<int>(a_value) == 12);
    REQUIRE(std::get<int>(b_value) == 20);
    REQUIRE(std::get<double>(c_value) == 2.0);
}

TEST_CASE("While loop", "[Codegen]")
{
    using namespace jl;

    const auto [temp_vars, var_map] = compile(R"( 
        var i = 0;
        var sum = 0;

        while (i <= 10) [
            sum += i;
            i += 1;
        ]
)");

    const auto i = temp_vars[var_map.at("i")];
    const auto sum = temp_vars[var_map.at("sum")];

    REQUIRE(std::get<int>(i) == 11);
    REQUIRE(std::get<int>(sum) == 55);
}

TEST_CASE("For loop", "[Codegen]")
{
    using namespace jl;

    const auto [temp_vars, var_map] = compile(R"(
        var sum = 0;

        for (var i = 0; i <= 10; i += 1) [
            sum += i;
        ]
)");

    const auto i = temp_vars[var_map.at("i")];
    const auto sum = temp_vars[var_map.at("sum")];

    REQUIRE(std::get<int>(i) == 11);
    REQUIRE(std::get<int>(sum) == 55);
}

TEST_CASE("If ladders", "[Codegen]")
{
    using namespace jl;

    const auto [temp_vars, var_map] = compile(R"(
        var a = 0;
        var b = 0;
        var c = 0;

        for (var i = 0; i < 3; i += 1) [            
            if (i == 0) [
                a = i;
            ] else if (i == 1) [
                b = i; 
            ] else [
                c = i; 
            ]
        ]
)");

    const auto a_value = temp_vars[var_map.at("a")];
    const auto b_value = temp_vars[var_map.at("b")];
    const auto c_value = temp_vars[var_map.at("c")];

    REQUIRE(std::get<int>(a_value) == 0);
    REQUIRE(std::get<int>(b_value) == 1);
    REQUIRE(std::get<int>(c_value) == 2);
}

TEST_CASE("Simple Function", "[Codegen]")
{
    using namespace jl;

    const auto [temp_vars, var_map] = compile(R"(
        fun sum_till(till: int): int [
            var sum = 0;

            for (var i = 1; i <= till; i+=1) [
                sum += i;
            ]

            return sum;
        ]

        var a = sum_till(10);
)");

    const auto a_value = temp_vars[var_map.at("a")];

    REQUIRE(std::get<int>(a_value) == 55);
}

TEST_CASE("Empty function", "[Codegen]")
{
    using namespace jl;

    const auto [temp_vars, var_map] = compile(R"(
        fun hai() [
        ]

        hai();
)");
}

TEST_CASE("Recursive function", "[Codegen]")
{
    using namespace jl;

    const auto [temp_vars, var_map] = compile(R"(
        fun factorial(n: int): int [
            if (n == 0) [
                return 1;
            ]

            return n * factorial(n - 1);
        ]

        var a = factorial(4 + 1);
)");

    const auto a_value = temp_vars[var_map.at("a")];

    REQUIRE(std::get<int>(a_value) == 120);
}

TEST_CASE("Fibonacci function", "[Codegen]")
{
    using namespace jl;

    const auto [temp_vars, var_map] = compile(R"(
fun fibonacci(n: int): int [
        var a = 0;
        var b = 1;

        if (n == 0 or n == 1) [
                return n;
        ]

        for(var i = 0; i < n; i += 1) [
                var c = a + b;
                a = b;
                b = c;
        ]

        return b;
]

var f = fibonacci(6);
)");

    const auto f_value = temp_vars[var_map.at("f")];

    REQUIRE(std::get<int>(f_value) == 13);
}

TEST_CASE("Test Charachters", "[Codegen]")
{
    using namespace jl;

    const auto [temp_vars, var_map] = compile(R"(
        var a = 'a';
        var b: char = '`';

        fun test_char(): char [
            return '+';
        ]

        var c = test_char();
)");

    const auto a_value = temp_vars[var_map.at("a")];
    const auto b_value = temp_vars[var_map.at("b")];
    const auto c_value = temp_vars[var_map.at("c")];

    REQUIRE(std::get<char>(a_value) == 'a');
    REQUIRE(std::get<char>(b_value) == '`');
    REQUIRE(std::get<char>(c_value) == '+');
}