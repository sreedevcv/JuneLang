#include "Parser.hpp"

#include <typeinfo>

#include "ErrorHandler.hpp"

jl::Parser::Parser(std::vector<Token>& tokens, std::string& file_name)
    : m_tokens(tokens)
    , m_file_name(file_name)
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

void jl::Parser::synchronize()
{
    advance();

    while (!is_at_end()) {
        if (previous().get_tokentype() == Token::SEMI_COLON)
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
        default:
            break;
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
        std::string error_where = "consuming a token (" + token.get_lexeme() + ")";
        ErrorHandler::error(m_file_name, "parsing", error_where.c_str(), token.get_line(), msg, 0);
        throw "parse-exception";
    }
}

// --------------------------------------------------------------------------------
// -------------------------------Expressions--------------------------------------
// --------------------------------------------------------------------------------

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

    return call();
}

jl::Expr* jl::Parser::primary()
{
    if (match({ Token::INT, Token::FLOAT, Token::STRING, Token::FALSE, Token::TRUE, Token::NULL_ })) {
        Value* value = new Value(previous().get_value());
        return new Literal(value);
    }

    if (match({ Token::IDENTIFIER })) {
        return new Variable(previous());
    }

    if (match({ Token::LEFT_PAR })) {
        Expr* expr = expression();
        consume(Token::RIGHT_PAR, "Expected ) after expression");
        return new Grouping(expr);
    }

    ErrorHandler::error(m_file_name, "parsing", "primary expression", peek().get_line(), "Expected a expression here", 0);
    throw "parse-exception";
}

jl::Expr* jl::Parser::assignment()
{
    Expr* expr = or_expr();

    if (match({ Token::EQUAL })) {
        Token& equals = previous();
        Expr* value = assignment();

        if (dynamic_cast<Variable*>(expr)) {
            Token& name = static_cast<Variable*>(expr)->m_name;
            return new Assign(value, name);
        } else if (dynamic_cast<Get*>(expr)) {
            Get* get_expr = static_cast<Get*>(expr);
            return new Set(get_expr->m_name, get_expr->m_object, value);
        }

        ErrorHandler::error(m_file_name, "parsing", "assignment", equals.get_line(), "Invalid assignment target, expected a variable", 0);
    }

    return expr;
}

jl::Expr* jl::Parser::or_expr()
{
    Expr* expr = and_expr();

    while (match({ Token::OR })) {
        Token& oper = previous();
        Expr* right = and_expr();
        expr = new Logical(expr, oper, right);
    }

    return expr;
}

jl::Expr* jl::Parser::and_expr()
{
    Expr* expr = equality();

    while (match({ Token::AND })) {
        Token& oper = previous();
        Expr* right = equality();
        expr = new Logical(expr, oper, right);
    }

    return expr;
}

jl::Expr* jl::Parser::call()
{
    Expr* expr = primary();

    while (true) {
        if (match({ Token::LEFT_PAR })) {
            expr = finish_call(expr);
        } else if (match({Token::DOT})) {
            Token& name = consume(Token::IDENTIFIER, "Expected property name after '.'");
            expr = new Get(name, expr);
        } else {
            break;
        }
    }

    return expr;
}

jl::Expr* jl::Parser::finish_call(Expr* callee)
{
    std::vector<Expr*> arguments;

    if (!check(Token::RIGHT_PAR)) {
        do {
            if (arguments.size() > 255) {
                ErrorHandler::error(m_file_name, "parsing", "function call", peek().get_line(), "Cannot have more than 255 args for a call", 0);
            }
            arguments.push_back(expression());
        } while (match({ Token::COMMA }));
    }

    Token& paren = consume(Token::RIGHT_PAR, "Expected ) after arguments");
    return new Call(callee, paren, arguments);
}

// --------------------------------------------------------------------------------
// -------------------------------Statements---------------------------------------
// --------------------------------------------------------------------------------

jl::Stmt* jl::Parser::statement()
{
    if (match({ Token::PRINT })) {
        return print_statement();
    }
    if (match({ Token::LEFT_SQUARE })) {
        return new BlockStmt(block());
    }
    if (match({ Token::IF })) {
        return if_stmt();
    }
    if (match({ Token::WHILE })) {
        return while_statement();
    }
    if (match({ Token::FOR })) {
        return for_statement();
    }
    if (match({Token::RETURN})) {
        return return_statement();
    }

    return expr_statement();
}

jl::Stmt* jl::Parser::declaration()
{
    try {
        if (match({Token::CLASS})) {
            return class_declaration();
        }
        if (match({ Token::FUNC })) {
            return function("function");
        }
        if (match({ Token::VAR })) {
            return var_declaration();
        }
        if (match({ Token::SEMI_COLON })) {
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
    consume(Token::SEMI_COLON, "Expected ; after expression");
    return new PrintStmt(expr);
}

jl::Stmt* jl::Parser::expr_statement()
{
    Expr* expr = expression();
    consume(Token::SEMI_COLON, "Expected ; after expression");
    return new ExprStmt(expr);
}

jl::Stmt* jl::Parser::var_declaration()
{
    Token& name = consume(Token::IDENTIFIER, "Expected a variable name");
    Expr* initializer = nullptr;

    if (match({ Token::EQUAL })) {
        initializer = expression();
    }

    consume(Token::SEMI_COLON, "Expected ; after variable declaration");
    return new VarStmt(name, initializer);
}

// NOTE::Unsure whether wee need a delinter after condition to properly parse
jl::Stmt* jl::Parser::if_stmt()
{
    Expr* condition = expression();
    // consume(Token::LEFT_SQUARE, "Expected [ after condition");
    Stmt* then_branch = statement();
    // consume(Token::RIGHT_SQUARE, "Expected ] after statements in if block");

    Stmt* else_branch = nullptr;

    if (match({ Token::ELSE })) {
        // consume(Token::LEFT_SQUARE, "Expected [ after condition");
        else_branch = statement();
        // consume(Token::RIGHT_SQUARE, "Expected ] after statements in if block");
    }

    return new IfStmt(condition, then_branch, else_branch);
}

jl::Stmt* jl::Parser::while_statement()
{
    Expr* condition = expression();
    Stmt* body = statement();

    return new WhileStmt(condition, body);
}

jl::Stmt* jl::Parser::for_statement()
{
    Stmt* initializer;
    if (match({ Token::SEMI_COLON })) {
        initializer = nullptr;
    } else if (match({ Token::VAR })) {
        initializer = var_declaration();
    } else {
        initializer = expr_statement();
    }

    Expr* condition = nullptr;
    if (!check(Token::SEMI_COLON)) {
        condition = expression();
    }
    consume(Token::SEMI_COLON, "Expected ; after loop condition");

    Expr* increment = nullptr;
    if (!check(Token::SEMI_COLON)) {
        increment = expression();
    }
    consume(Token::SEMI_COLON, "Expected ; after all loop clauses");

    Stmt* body = statement();

    if (increment != nullptr) {
        body = new BlockStmt({ body, new ExprStmt(increment) });
    }
    if (condition == nullptr) {
        condition = new Literal(&Token::global_true_constant);
    }
    body = new WhileStmt(condition, body);

    if (initializer != nullptr) {
        body = new BlockStmt({ initializer, body });
    }

    return body;
}

jl::Stmt* jl::Parser::function(const char* kind)
{
    Token& name = consume(Token::IDENTIFIER, "Expeced a function name here");

    consume(Token::LEFT_PAR, "Expected ( after fun name");
    std::vector<Token*> parameters;

    if (!check(Token::RIGHT_PAR)) {
        do {
            if (parameters.size() >= 255) {
                ErrorHandler::error(m_file_name, "parsing", "function call", peek().get_line(), "Cannot have more than 255 parameters for a function", 0);
            }
            Token& param = consume(Token::IDENTIFIER, "Expected parameter name here");
            parameters.push_back(&param);

        } while (match({Token::COMMA}));
    }

    consume(Token::RIGHT_PAR, "Expected ) after function parameters");
    consume(Token::LEFT_SQUARE, "Expected [ befor function body");

    std::vector<Stmt*> body = block();
    return new FuncStmt(name, parameters, body);
}

jl::Stmt* jl::Parser::return_statement()
{
    Token& return_token = previous();
    Expr* expr = nullptr;

    if (!check(Token::SEMI_COLON)) {
        expr = expression();    
    }

    consume(Token::SEMI_COLON, "Expected ; after return");

    return new ReturnStmt(return_token, expr);
}

jl::Stmt* jl::Parser::class_declaration()
{
    Token& name = consume(Token::IDENTIFIER, "Expected a class name");
    consume(Token::LEFT_SQUARE, "Expected a [ before class body");

    std::vector<FuncStmt*> methods;
    while (!check(Token::RIGHT_SQUARE) && !is_at_end()) {
        methods.push_back(static_cast<FuncStmt*>(function("method")));
    }

    consume(Token::RIGHT_SQUARE, "Expected a ] after class body");
    return new ClassStmt(name, methods);
}

std::vector<jl::Stmt*> jl::Parser::block()
{
    std::vector<Stmt*> statements;

    while (!check(Token::RIGHT_SQUARE) && !is_at_end()) {
        statements.push_back(declaration());
    }

    consume(Token::RIGHT_SQUARE, "Expected ] after block");
    return statements;
}
