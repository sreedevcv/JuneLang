#include <catch2/catch_test_macros.hpp>

#include <iostream>
#include <string>
#include <vector>

#include "Expr.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "ErrorHandler.hpp"
#include "Arena.hpp"

TEST_CASE("Parser Expression Test", "[Parser]")
{
    // std::string file = "../../tests/scripts/parser_test_1.jun";
    // jl::Lexer lexer(file);
    // lexer.scan();
    // auto tokens = lexer.get_tokens();

    // if (!jl::ErrorHandler::has_error()) {

    //     REQUIRE(tokens.size() != 0);
    //     jl::Arena arena(1000);

    //     jl::Parser parser(arena, tokens, file);
    //     jl::Expr* e = parser.parse();

    //     REQUIRE(e != nullptr);

    //     jl::IExprVisitor* visitor = new jl::ParsetreePrinter();
    //     e->accept(*visitor, visitor->get_expr_context());
    //     std::string* context = (std::string*)visitor->get_expr_context();
    //     REQUIRE(*context == std::string("(== (group (+ 12 (* (group (- 32 9)) 5))) (<= 34 (group (/ 7 8))))"));
    // }
}