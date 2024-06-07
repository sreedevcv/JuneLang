#pragma once

#include <string>
#include <variant>

namespace jl {
class Token {
public:
    enum TokenType {
        // Single Charachter
        COMMA,
        DOT,
        PLUS,
        MINUS,
        STAR,
        SLASH,
        LEFT_BRACE,
        RIGHT_BRACE,
        LEFT_SQUARE,
        RIGHT_SQUARE,
        LEFT_PAR,
        RIGHT_PAR,
        SEMI_COLON,
        END_OF_FILE,
        // One or Two Characters
        BANG,
        EQUAL,
        BANG_EQUAL,
        EQUAL_EQUAL,
        GREATER,
        GREATER_EQUAL,
        LESS,
        LESS_EQUAL,
        // Literals
        STRING,
        NUMBER,
        IDENTIFIER,
        // Keywords
        AND,
        OR,
        NOT,
        IF,
        THEN,
        WHILE,
        DO,
        FOR,
        END,
        TRUE,
        FALSE,
        RETURN,
        VAR,
    };

    using Value = std::variant<int, float, std::string>;

    Token(TokenType type, std::string& lexeme, int line);
    Token(TokenType type, std::string& lexeme, Value value, int line);
    ~Token();

    private:
        TokenType m_type;
        std::string m_lexem;
        int m_line;
        Value m_value;
};
} // namespace jl
