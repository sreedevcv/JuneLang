#include <catch2/catch_test_macros.hpp>

#include "Lexer.hpp"
#include <string>

#include <iostream>

#include <limits.h>
    #include <unistd.h>

TEST_CASE("Lexer Scan", "[Lexer]") {
    
    std::string path = "../../tests/scripts/lexer_test.jun";
    jl::Lexer lexer(path);
    lexer.scan();
    REQUIRE(0 == 0);
}