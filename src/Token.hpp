#pragma once

#include <string>
#include <variant>

#include "Value.hpp"

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
        NEW_LINE,
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
        FLOAT,
        INT,
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
        DO,
        FOR,
        END,
        TRUE,
        FALSE,
        RETURN,
        VAR,
        PRINT,
        NULL_,
        THIS,
    };

    Token(TokenType type, std::string& lexeme, int line);
    Token(TokenType type, std::string& lexeme, int line, Value value);
    ~Token();

    TokenType get_tokentype() const;
    std::string& get_lexeme();
    const Value get_value() const;
    int get_line() const;

    static Value global_true_constant;
    static Value global_false_constant;

private:
    TokenType m_type;
    std::string m_lexeme;
    int m_line;
    Value m_value;
};

bool is_int(Value& value);
bool is_float(Value& value);
bool is_bool(Value& value);
bool is_string(Value& value);
bool is_null(Value& value);
bool is_number(Value& value);
bool is_callable(Value& value);
bool is_instance(Value& value);
} // namespace jl
