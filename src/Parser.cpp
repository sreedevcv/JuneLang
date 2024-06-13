#include "Parser.hpp"

#include <typeinfo>

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

std::vector<jl::Stmt*> jl::Parser::parseStatements()
{
    std::vector<Stmt*> statements;
    try {
        while (!is_at_end()) {
            statements.push_back(declaration());
        }
    } catch (const char* e) {
    }

    return statements;
}

jl::Expr* jl::Parser::expression()
{
    return assignment();
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

    if (match({Token::IDENTIFIER})) {
        return new Variable(previous());
    }

    if (match({ Token::LEFT_PAR })) {
        Expr* expr = expression();
        consume(Token::RIGHT_PAR, "Expect ) after expression");
        return new Grouping(expr);
    }

    // TODO::Improve error function
    std::string file = std::string("Unknown");
    ErrorHandler::error(file, peek().get_line(), "Expected expression");
    throw "parse-exception";
}

jl::Expr* jl::Parser::assignment()
{
    Expr* expr = equality();

    if (match({Token::EQUAL})) {
        Token& equals = previous();
        Expr* value = assignment();

        if (dynamic_cast<Variable*>(expr)) {
            Token& name = static_cast<Variable*>(expr)->m_name;
            return new Assign(value, name);
        }

        std::string filename = "Unknown";
        ErrorHandler::error(filename, equals.get_line(), "invalid assignment target");
    }

    return expr;
}

void jl::Parser::synchronize()
{
    advance();

    while (!is_at_end()) {
        if (previous().get_tokentype() == Token::NEW_LINE)
            return;

        switch (peek().get_tokentype()) {
        case Token::CLASS:
        case Token::FUNC:
        case Token::VAR:
        case Token::FOR:
        case Token::IF:
        case Token::WHILE:
        case Token::PRINT:
        case Token::RETURN:
            return;
        }

        advance();
    }
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
        throw "parse-exception";
    }
}

jl::Stmt* jl::Parser::statement()
{
    if (match({ Token::PRINT })) {
        return print_statement();
    }
    if (match({Token::LEFT_SQUARE})) {
        return new BlockStmt(block());
    }

    return expr_statement();
}

jl::Stmt* jl::Parser::declaration()
{
    try {
        if (match({ Token::VAR })) {
            return var_declaration();
        }
        if (match({Token::NEW_LINE})) {
            return new EmptyStmt();
        }
        return statement();
    } catch (const char* e) {
        synchronize();
        return nullptr;
    }
}

jl::Stmt* jl::Parser::print_statement()
{
    Expr* expr = expression();
    consume(Token::NEW_LINE, "Expected newline after expression");
    return new PrintStmt(expr);
}

jl::Stmt* jl::Parser::expr_statement()
{
    Expr* expr = expression();
    consume(Token::NEW_LINE, "Expected newline after expression");
    return new ExprStmt(expr);
}

jl::Stmt* jl::Parser::var_declaration()
{
    Token& name = consume(Token::IDENTIFIER, "Expected a variable name");
    Expr* initializer = nullptr;

    if (match({Token::EQUAL})) {
        initializer = expression();
    }

    consume(Token::NEW_LINE, "Expected newline after variable declaration");
    return new VarStmt(name, initializer);
}

std::vector<jl::Stmt*> jl::Parser::block()
{
    std::vector<Stmt*> statements;

    while (!check(Token::RIGHT_SQUARE) && !is_at_end()) {
        statements.push_back(declaration());
    }

    consume(Token::RIGHT_SQUARE, "Expect ] after block");
    return statements;
}
