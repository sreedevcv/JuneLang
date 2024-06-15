#include <catch2/catch_test_macros.hpp>

#include <iostream>
#include <string>
#include <vector>

#include "ErrorHandler.hpp"
#include "Interpreter.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
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