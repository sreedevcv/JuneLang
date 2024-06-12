#include <catch2/catch_test_macros.hpp>

#include <iostream>
#include <string>
#include <vector>

#include "Lexer.hpp"
#include "Token.hpp"

TEST_CASE("Lexer Scan 1", "[Lexer]")
{
    using namespace jl;

    std::string path = "../../tests/scripts/lexer_test_1.jun";
    Lexer lexer(path);
    lexer.scan();
    std::vector<Token> scanned_tokens = lexer.get_tokens();
    std::vector<Token::TokenType> expected_tokens = { Token::STAR, Token::EQUAL, Token::PLUS, Token::BANG, Token::MINUS, Token::SEMI_COLON, Token::LESS, Token::GREATER, Token::LESS_EQUAL, Token::GREATER_EQUAL, Token::BANG_EQUAL, Token::EQUAL_EQUAL, Token::END_OF_FILE };

    REQUIRE(scanned_tokens.size() == expected_tokens.size());

    for (int i = 0; i < scanned_tokens.size(); i++) {
        REQUIRE(scanned_tokens[i].get_tokentype() == expected_tokens[i]);
    }
}

TEST_CASE("Lexer Scan 2", "[Lexer]")
{
    using namespace jl;

    std::string path = "../../tests/scripts/lexer_test_2.jun";
    Lexer lexer(path);
    lexer.scan();
    std::vector<Token> scanned_tokens = lexer.get_tokens();
    std::vector<Token::TokenType> expected_tokens = { Token::STAR, Token::STAR, Token::PLUS, Token::MINUS, Token::NEW_LINE, Token::NEW_LINE, Token::NEW_LINE,
        Token::NEW_LINE, Token::NEW_LINE, Token::MINUS, Token::NEW_LINE,
        Token::STRING, Token::NEW_LINE, Token::NEW_LINE, Token::PLUS, Token::NEW_LINE,
        Token::NEW_LINE, Token::INT, Token::NEW_LINE, Token::FLOAT, Token::NEW_LINE,
        Token::NEW_LINE, Token::IDENTIFIER, Token::IDENTIFIER, Token::AND, Token::OR, Token::NEW_LINE,
        Token::IDENTIFIER, Token::IF, Token::NEW_LINE,
        Token::END_OF_FILE };

    REQUIRE(scanned_tokens.size() == expected_tokens.size());

    for (int i = 0; i < scanned_tokens.size(); i++) {
        REQUIRE(scanned_tokens[i].get_tokentype() == expected_tokens[i]);

        switch (scanned_tokens[i].get_tokentype()) {
        case Token::STRING:
            REQUIRE(std::get<std::string>(scanned_tokens[i].get_value()) == "hello+==");
            break;
        case Token::INT:
            REQUIRE(std::get<int>(scanned_tokens[i].get_value()) == 123);
            break;
        case Token::FLOAT:
            REQUIRE(std::get<double>(scanned_tokens[i].get_value()) == std::stod("434.534"));
            break;
        default:
            break;
        }
    }
}

TEST_CASE("Lexer Keyword Scan", "[Lexer]")
{
    using namespace jl;

    std::string path = "../../tests/scripts/lexer_test_3.jun";
    Lexer lexer(path);
    lexer.scan();
    std::vector<Token> scanned_tokens = lexer.get_tokens();
    std::vector<Token::TokenType> expected_tokens = { Token::IDENTIFIER, Token::FOR, Token::IDENTIFIER, Token::IF, Token::NEW_LINE, Token::AND, Token::IDENTIFIER, Token::OR, Token::IDENTIFIER, Token::NOT, Token::NEW_LINE, Token::INT, Token::WHILE, Token::NEW_LINE, Token::THEN, Token::END, Token::FLOAT, Token::TRUE, Token::IDENTIFIER, Token::NEW_LINE, Token::IDENTIFIER, Token::FALSE, Token::LEFT_PAR, Token::RIGHT_PAR, Token::LEFT_BRACE, Token::RIGHT_BRACE, Token::RETURN, Token::VAR, Token::NEW_LINE, Token::NULL_, Token::NEW_LINE, Token::NEW_LINE, Token::END_OF_FILE };

    REQUIRE(scanned_tokens.size() == expected_tokens.size());

    for (int i = 0; i < scanned_tokens.size(); i++) {
        REQUIRE(scanned_tokens[i].get_tokentype() == expected_tokens[i]);
    }
}
