#include "Parser.hpp"

#include "ErrorHandler.hpp"

jl::Parser::Parser(std::vector<Token>& tokens)
    : m_tokens(tokens)
{
}

jl::Expr* jl::Parser::parse()
{
    try {
        return expression();
    } catch (const char* e) {
        return nullptr;
    }
}

jl::Expr* jl::Parser::expression()
{
    return equality();
}

jl::Expr* jl::Parser::equality()
{
    Expr* expr = comparison();

    while (match({ Token::BANG_EQUAL, Token::EQUAL_EQUAL })) {
        Token& oper = previous();
        Expr* right = comparison();
        expr = new Binary(expr, &oper, right);
    }
    return expr;
}

jl::Expr* jl::Parser::comparison()
{
    Expr* expr = term();

    while (match({ Token::GREATER, Token::GREATER_EQUAL, Token::LESS, Token::LESS_EQUAL })) {
        Token& oper = previous();
        Expr* right = term();
        expr = new Binary(expr, &oper, right);
    }
    return expr;
}

jl::Expr* jl::Parser::term()
{
    Expr* expr = factor();

    while (match({ Token::MINUS, Token::PLUS })) {
        Token& oper = previous();
        Expr* right = factor();
        expr = new Binary(expr, &oper, right);
    }
    return expr;
}

jl::Expr* jl::Parser::factor()
{
    Expr* expr = unary();

    while (match({ Token::SLASH, Token::STAR })) {
        Token& oper = previous();
        Expr* right = unary();
        expr = new Binary(expr, &oper, right);
    }
    return expr;
}

jl::Expr* jl::Parser::unary()
{
    if (match({ Token::BANG, Token::MINUS })) {
        Token& oper = previous();
        Expr* right = unary();
        return new Unary(&oper, right);
    }
    return primary();
}

jl::Expr* jl::Parser::primary()
{
    if (match({ Token::INT, Token::FLOAT, Token::STRING, Token::FALSE, Token::TRUE, Token::NULL_ })) {
        Token::Value* value = new Token::Value(previous().get_value());
        return new Literal(value);
    }

    if (match({ Token::LEFT_PAR })) {
        Expr* expr = expression();
        consume(Token::RIGHT_PAR, "Expect ) after expression");
        return new Grouping(expr);
    }

    // TODO::Improve error function
    std::string file = std::string("Unknown");
    ErrorHandler::error(file, peek().get_line(), "Expected expression");
    throw "exception";
}

bool jl::Parser::match(std::vector<Token::TokenType>&& types)
{
    for (const auto type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

bool jl::Parser::check(Token::TokenType type)
{
    return is_at_end() ? false : peek().get_tokentype() == type;
}

bool jl::Parser::is_at_end()
{
    return peek().get_tokentype() == Token::END_OF_FILE;
}

jl::Token& jl::Parser::advance()
{
    if (!is_at_end()) {
        m_current++;
    }
    return previous();
}

jl::Token& jl::Parser::peek()
{
    return m_tokens[m_current];
}

jl::Token& jl::Parser::previous()
{
    return m_tokens[m_current - 1];
}

jl::Token& jl::Parser::consume(Token::TokenType type, const char* msg)
{
    if (check(type)) {
        return advance();
    } else {
        Token& token = peek();
        // TODO::Improve error function
        std::string file = std::string("Unknown");
        ErrorHandler::error(file, token.get_line(), msg);
        throw "exception";
    }
}
