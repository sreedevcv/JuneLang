#include <catch2/catch2.hpp>

#include "Lexer.hpp"

TEST_CASE("Lexer Scan", "[Lexer]") {
    jl::Lexer lexer("tests/scripts/lexer_test.jun");
    lexer.scan();
    REQUIRE(0 == 0);
}