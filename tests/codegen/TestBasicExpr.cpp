#include "VM.hpp"
#include "catch2/catch_test_macros.hpp"

#include "CodeGenerator.hpp"
#include "Lexer.hpp"
#include "CodeGenerator.hpp"
#include "ErrorHandler.hpp"
#include "Interpreter.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "Resolver.hpp"


TEST_CASE("Expressions", "[Codegen]")
{
    using namespace jl;

    jl::Lexer lexer(
        R"( var a = 10 + 2;
        var b = (a * 2) - (3 + 1);
        var c = b / 10.0;
)");

    std::string file_name = "test.jun";

    lexer.scan();

    REQUIRE(jl::ErrorHandler::has_error() == false);
    

    auto tokens = lexer.get_tokens();
    jl::Parser parser(tokens, file_name);
    auto stmts = parser.parseStatements();

    jl::Interpreter interpreter(file_name);
    jl::Resolver resolver(interpreter, file_name);
    resolver.resolve(stmts);

    REQUIRE(jl::ErrorHandler::has_error() == false);

    jl::CodeGenerator codegen(file_name);
    codegen.generate(stmts);
    const auto chunk = codegen.get_chunk();

    jl::VM vm;
    const auto [status, temp_vars] = vm.run(chunk);
    const auto var_map = chunk.get_variable_map();

    const auto a_value = temp_vars[var_map.at("a").idx];
    const auto b_value = temp_vars[var_map.at("b").idx];
    const auto c_value = temp_vars[var_map.at("c").idx];

    REQUIRE(std::get<int>(a_value) == 12);
    REQUIRE(std::get<int>(b_value) == 20);
    REQUIRE(std::get<double>(c_value) == 2.0);
}