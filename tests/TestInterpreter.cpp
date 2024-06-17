#include <catch2/catch_test_macros.hpp>

#include <iostream>
#include <string>
#include <vector>

#include "ErrorHandler.hpp"
#include "Interpreter.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "Resolver.hpp"
#include "Token.hpp"

TEST_CASE("Interpreter Simple Expressions", "[Interpreter]")
{
    const int len = 6;
    const char* expressions[] = {
        "\"hello \" + \"hai\"",
        "1 + 2",
        "(1.5 * 2) + 3",
        "(5 + 6.5) / 2 > 1.5 == false",
        "null != true",
        "null == null",
    };

    std::string expected[] = {
        "hello hai",
        "3",
        "6.000000",
        "false",
        "true",
        "true"
    };

    std::string file_name = "TestInterpreter";

    for (int i = 0; i < len; i++) {
        jl::Lexer lexer(expressions[i]);
        lexer.scan();

        REQUIRE(!jl::ErrorHandler::has_error());
        lexer.scan();
        auto tokens = lexer.get_tokens();
        REQUIRE(tokens.size() != 0);

        jl::Parser parser(tokens, file_name);
        jl::Expr* e = parser.parse();
        REQUIRE(e != nullptr);

        jl::Value value;
        jl::Interpreter interpreter(file_name);
        interpreter.interpret(e, &value);
        std::string result = interpreter.stringify(value);
        REQUIRE(result == expected[i]);
    }
}

void test_string(const char* source)
{
    jl::Lexer lexer(source);
    jl::ErrorHandler::clear_errors();
    jl::ErrorHandler::m_stream.setOutputToFile("../../tests/scripts/temp.txt");

    std::string file_name = "test";
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

    jl::Value v;
    interpreter.interpret(stmts);
    REQUIRE(jl::ErrorHandler::has_error() == false);
}

TEST_CASE("Interpreter Recursive Function", "[Interpreter]")
{
    const char* source = R"(
        fun fib(n) [
            if n <= 1 return n;
            return fib(n - 2) + fib(n - 1);
        ]

        for var i = 0; i < 20; i = i + 1; [
            print fib(i);
        ]
    )";

    test_string(source);
}

TEST_CASE("Interpreter Binding", "[Interpreter]")
{
    const char* source = R"(
        var a = "global";
        [
            fun showA() [
                print a;
            ]

            showA();
            var a = "block";
            showA();
        ]
    )";

    test_string(source);
}

TEST_CASE("Interpreter Declaration with the same variable", "[Interpreter]")
{
    const char* source = R"(
        var a = "outer";
        [
            var a = a;
        ]
    )";

    jl::Lexer lexer(source);
    jl::ErrorHandler::clear_errors();
    jl::ErrorHandler::m_stream.setOutputToFile("../../tests/scripts/temp.txt");

    std::string file_name = "test";
    lexer.scan();
    REQUIRE(jl::ErrorHandler::has_error() == false);

    auto tokens = lexer.get_tokens();
    jl::Parser parser(tokens, file_name);
    auto stmts = parser.parseStatements();
    REQUIRE(jl::ErrorHandler::has_error() == false);

    jl::Interpreter interpreter(file_name);
    jl::Resolver resolver(interpreter, file_name);
    resolver.resolve(stmts);

    // Errors should occur
    REQUIRE(jl::ErrorHandler::has_error() == true);
}