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
        SUPER,
        BREAK,
    };

    Token(TokenType type, std::string& lexeme, int line);
    Token(TokenType type, std::string& lexeme, int line, JlValue* value);
    ~Token();

    TokenType get_tokentype() const;
    std::string& get_lexeme();
    JlValue* get_value() const;
    int get_line() const;

    static JlBool global_true_constant;
    static JlBool global_false_constant;
    static std::string global_super_lexeme;
    static std::string global_this_lexeme;
    // static Token global_plus_equal;

private:
    TokenType m_type;
    std::string m_lexeme;
    int m_line;
    JlValue* m_value;
};

// bool is_int(JlValue* value);
// bool is_float(JlValue* value);
// bool is_bool(JlValue* value);
// bool is_string(JlValue* value);
// bool is_null(JlValue* value);
// bool is_number(JlValue* value);
// bool is_callable(JlValue* value);
// bool is_instance(JlValue* value);
// bool is_jlist(JlValue* value);
} // namespace jl
