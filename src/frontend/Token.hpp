#pragma once

#include <string>

#include "Value.hpp"

namespace jl {
class Token : public Ref {
public:
    enum TokenType {
        // Single Charachter
        COMMA,
        DOT,
        COLON,
        STAR,
        SLASH,
        LEFT_BRACE,
        RIGHT_BRACE,
        LEFT_SQUARE,
        RIGHT_SQUARE,
        LEFT_PAR,
        RIGHT_PAR,
        SEMI_COLON,
        NEW_LINE,
        END_OF_FILE,
        // One or Two Characters
        PLUS,
        BANG,
        LESS,
        EQUAL,
        MINUS,
        GREATER,
        PERCENT,
        PLUS_EQUAL,
        MINUS_EQUAL,
        STAR_EQUAL,
        SLASH_EQUAL,
        BANG_EQUAL,
        EQUAL_EQUAL,
        GREATER_EQUAL,
        LESS_EQUAL,
        PERCENT_EQUAL,
        BIT_AND,
        BIT_OR,
        BIT_XOR,
        BIT_NOT,
        // Literals
        STRING,
        FLOAT,
        INT,
        CHAR,
        IDENTIFIER,
        // Keywords
        AND,
        OR,
        NOT,
        CLASS,
        FUNC,
        IF,
        THEN,
        ELSE,
        WHILE,
        FOR,
        TRUE,
        FALSE,
        RETURN,
        VAR,
        PRINT,
        NULL_,
        THIS,
        SUPER,
        BREAK,
        EXTERN,
        AS
    };

    Token(TokenType type, std::string& lexeme, int line);
    Token(TokenType type, std::string& lexeme, int line, Value* value);
    ~Token();

    TokenType get_tokentype() const;
    std::string& get_lexeme();
    Value* get_value() const;
    int get_line() const;

    static Value global_true_constant;
    static Value global_false_constant;
    static std::string global_super_lexeme;
    static std::string global_this_lexeme;
    static Token global_void_token;
    // static Token global_plus_equal;

private:
    TokenType m_type;
    std::string m_lexeme;
    int m_line;
    Value* m_value;
};

} // namespace jl
