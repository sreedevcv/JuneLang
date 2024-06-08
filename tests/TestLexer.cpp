#include <catch2/catch_test_macros.hpp>

#include "Lexer.hpp"
#include "Token.hpp"
#include <string>
#include <vector>

TEST_CASE("Lexer Scan", "[Lexer]")
{
    using namespace jl;

    std::string path = "../../tests/scripts/lexer_test.jun";
    Lexer lexer(path);
    lexer.scan();
    std::vector<Token> scanned_tokens = lexer.get_tokens();
    std::vector<Token::TokenType> expected_tokens = { Token::STAR, Token::EQUAL, Token::PLUS, Token::BANG, Token::MINUS, Token::SEMI_COLON, Token::LESS, Token::GREATER, Token::LESS_EQUAL, Token::GREATER_EQUAL, Token::BANG_EQUAL, Token::EQUAL_EQUAL };

    REQUIRE(scanned_tokens.size() == expected_tokens.size());

    for (int i = 0; i < scanned_tokens.size(); i++) {
        REQUIRE(scanned_tokens[i].get_tokentype() == expected_tokens[i]);
    }
}