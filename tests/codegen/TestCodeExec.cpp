#include "Operand.hpp"
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

TEST_CASE("Index Get: Frequency Count", "[Codegen]")
{
    using namespace jl;

    const auto [temp_vars, var_map] = compile(R"(
        fun find_frequency(str: char_ptr, size: int, target: char): int [
            var count = 0;

            for (var i = 0; i < size; i += 1) [
                if (str[i] == target) [
                    count += 1;
                ]
            ]

            return count;
        ]

        var str = "Malayalam";
        var size = 9;
        var t1 = 'y';
        var t2 = 'a';
        var t3 = 'z';

        var f1 = find_frequency(str, size, t1);
        var f2 = find_frequency(str, size, t2);
        var f3 = find_frequency(str, size, t3);
)");

    const auto f1_value = temp_vars[var_map.at("f1")];
    const auto f2_value = temp_vars[var_map.at("f2")];
    const auto f3_value = temp_vars[var_map.at("f3")];

    REQUIRE(std::get<int>(f1_value) == 1);
    REQUIRE(std::get<int>(f2_value) == 4);
    REQUIRE(std::get<int>(f3_value) == 0);
}

TEST_CASE("Index Set: Replace Char Count", "[Codegen]")
{
    using namespace jl;

    const auto [temp_vars, var_map] = compile(R"(
        fun replace_char(str: char_ptr, size: int, target: char, new_value: char): int [
            var count = 0;

            for (var i = 0; i < size; i += 1) [
                if (str[i] == target) [
                    str[i] = new_value;
                    count += 1;
                ]
            ]

            return count;
        ]

        var str = "haaai";
        var count1 = replace_char(str, 5, 'a', 'e');
        var count2 = replace_char(str, 5, 'a', 'e');

        str[2] = 'a';

        var count3 = replace_char(str, 5, 'a', 'e');
)");

    const auto count1_value = temp_vars[var_map.at("count1")];
    const auto count2_value = temp_vars[var_map.at("count2")];
    const auto count3_value = temp_vars[var_map.at("count3")];

    REQUIRE(std::get<int>(count1_value) == 3);
    REQUIRE(std::get<int>(count2_value) == 0);
    REQUIRE(std::get<int>(count3_value) == 1);
}