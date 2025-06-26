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
    const auto chunk_map =codegen.generate(stmts);

    REQUIRE(jl::ErrorHandler::has_error() == false);

    jl::VM vm;
    const auto chunk = codegen.get_root_chunk();
    const auto [status, temp_vars] = vm.run(chunk, chunk_map);
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