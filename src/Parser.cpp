#include "Parser.hpp"

#include "ErrorHandler.hpp"
#include "Expr.hpp"

jl::Parser::Parser(Arena& arena, std::vector<Token>& tokens, std::string& file_name)
    : m_arena(arena)
    , m_tokens(tokens)
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

jl::Expr* jl::Parser::parse_list()
{
    // Token& curr = advance();
    std::vector<Expr*> list;
    
    while (!is_at_end() && peek().get_tokentype() != Token::RIGHT_BRACE) {
        Token& token = peek();

        // if (token.get_tokentype() == Token::LEFT_BRACE) {
        // if (match({Token::LEFT_BRACE}))
        //     Expr* list_expr = parse_list();
        //     list.push_back(list_expr);
        // } else {
            Expr* expr = or_expr();
            list.push_back(expr);
            consume(Token::COMMA, "Lists hould be comma seperated");
        // }
    }

    consume(Token::RIGHT_BRACE, "Lists should end with '}'");

    return m_arena.allocate<JList>(list);
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
        expr = m_arena.allocate<Binary>(expr, &oper, right);
    }
    return expr;
}

jl::Expr* jl::Parser::comparison()
{
    Expr* expr = term();

    while (match({ Token::GREATER, Token::GREATER_EQUAL, Token::LESS, Token::LESS_EQUAL })) {
        Token& oper = previous();
        Expr* right = term();
        expr = m_arena.allocate<Binary>(expr, &oper, right);
    }
    return expr;
}

jl::Expr* jl::Parser::term()
{
    Expr* expr = factor();

    while (match({ Token::MINUS, Token::PLUS })) {
        Token& oper = previous();
        Expr* right = factor();
        expr = m_arena.allocate<Binary>(expr, &oper, right);
    }
    return expr;
}

jl::Expr* jl::Parser::factor()
{
    Expr* expr = unary();

    while (match({ Token::SLASH, Token::STAR })) {
        Token& oper = previous();
        Expr* right = unary();
        expr = m_arena.allocate<Binary>(expr, &oper, right);
    }
    return expr;
}

jl::Expr* jl::Parser::unary()
{
    if (match({ Token::BANG, Token::MINUS })) {
        Token& oper = previous();
        Expr* right = unary();
        return m_arena.allocate<Unary>(&oper, right);
    }

    return call();
}

jl::Expr* jl::Parser::primary()
{
    if (match({ Token::INT, Token::FLOAT, Token::STRING, Token::FALSE, Token::TRUE, Token::NULL_ })) {
        Value* value = m_arena.allocate<Value>(previous().get_value());
        return m_arena.allocate<Literal>(value);
    }
    if (match({ Token::THIS })) {
        return m_arena.allocate<This>(previous());
    }
    if (match({ Token::IDENTIFIER })) {
        return m_arena.allocate<Variable>(previous());
    }
    if (match({ Token::LEFT_PAR })) {
        Expr* expr = expression();
        consume(Token::RIGHT_PAR, "Expected ) after expression");
        return m_arena.allocate<Grouping>(expr);
    }
    if (match({ Token::SUPER })) {
        Token& keyword = previous();
        consume(Token::DOT, "Expected '.' after super");
        Token& method = consume(Token::IDENTIFIER, "Expect superclass method name");
        return m_arena.allocate<Super>(keyword, method);
    }
    if (match({ Token::LEFT_BRACE })) {
        return parse_list();
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
            return m_arena.allocate<Assign>(value, name);
        } else if (dynamic_cast<Get*>(expr)) {
            Get* get_expr = static_cast<Get*>(expr);
            return m_arena.allocate<Set>(get_expr->m_name, get_expr->m_object, value);
        }

        ErrorHandler::error(m_file_name, "parsing", "assignment", equals.get_line(), "Invalid assignment target, expected a variable", 0);
    } else if (match({ Token::PLUS_EQUAL })) {
        return modify_and_assign(Token::PLUS, expr);
    } else if (match({ Token::MINUS_EQUAL })) {
        return modify_and_assign(Token::MINUS, expr);
    } else if (match({ Token::STAR_EQUAL })) {
        return modify_and_assign(Token::STAR, expr);
    } else if (match({ Token::SLASH_EQUAL })) {
        return modify_and_assign(Token::SLASH, expr);
    }

    return expr;
}

jl::Expr* jl::Parser::or_expr()
{
    Expr* expr = and_expr();

    while (match({ Token::OR })) {
        Token& oper = previous();
        Expr* right = and_expr();
        expr = m_arena.allocate<Logical>(expr, oper, right);
    }

    return expr;
}

jl::Expr* jl::Parser::and_expr()
{
    Expr* expr = equality();

    while (match({ Token::AND })) {
        Token& oper = previous();
        Expr* right = equality();
        expr = m_arena.allocate<Logical>(expr, oper, right);
    }

    return expr;
}

jl::Expr* jl::Parser::call()
{
    Expr* expr = primary();

    while (true) {
        if (match({ Token::LEFT_PAR })) {
            expr = finish_call(expr);
        } else if (match({ Token::DOT })) {
            Token& name = consume(Token::IDENTIFIER, "Expected property name after '.'");
            expr = m_arena.allocate<Get>(name, expr);
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
    return m_arena.allocate<Call>(callee, paren, arguments);
}

jl::Expr* jl::Parser::modify_and_assign(Token::TokenType oper_type, Expr* expr)
{
    Token& oper_equals = previous();
    Expr* value = or_expr();

    if (dynamic_cast<Variable*>(expr)) {
        Token& name = static_cast<Variable*>(expr)->m_name;
        Token* oper_token = m_arena.allocate<Token>(oper_type, previous().get_lexeme(), previous().get_line());
        Binary* oper = m_arena.allocate<Binary>(expr, oper_token, value);
        return m_arena.allocate<Assign>(oper, name);
    } else if (dynamic_cast<Get*>(expr)) {
        Get* get_expr = static_cast<Get*>(expr);
        Token* oper_token = m_arena.allocate<Token>(oper_type, previous().get_lexeme(), previous().get_line());
        Binary* oper = m_arena.allocate<Binary>(expr, oper_token, value);
        return m_arena.allocate<Set>(get_expr->m_name, get_expr->m_object, oper);
    }

    ErrorHandler::error(m_file_name, "parsing", "add assignment", oper_equals.get_line(), "Invalid assignment target, expected a variable", 0);
    return expr;
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
        return m_arena.allocate<BlockStmt>(block());
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
    if (match({ Token::RETURN })) {
        return return_statement();
    }

    return expr_statement();
}

jl::Stmt* jl::Parser::declaration()
{
    try {
        if (match({ Token::CLASS })) {
            return class_declaration();
        }
        if (match({ Token::FUNC })) {
            return function("function");
        }
        if (match({ Token::VAR })) {
            return var_declaration();
        }
        if (match({ Token::SEMI_COLON })) {
            return m_arena.allocate<EmptyStmt>();
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
    return m_arena.allocate<PrintStmt>(expr);
}

jl::Stmt* jl::Parser::expr_statement()
{
    Expr* expr = expression();
    consume(Token::SEMI_COLON, "Expected ; after expression");
    return m_arena.allocate<ExprStmt>(expr);
}

jl::Stmt* jl::Parser::var_declaration()
{
    Token& name = consume(Token::IDENTIFIER, "Expected a variable name");
    Expr* initializer = nullptr;

    if (match({ Token::EQUAL })) {
        initializer = expression();
    }

    consume(Token::SEMI_COLON, "Expected ; after variable declaration");
    return m_arena.allocate<VarStmt>(name, initializer);
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

    return m_arena.allocate<IfStmt>(condition, then_branch, else_branch);
}

jl::Stmt* jl::Parser::while_statement()
{
    Expr* condition = expression();
    Stmt* body = statement();

    return m_arena.allocate<WhileStmt>(condition, body);
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
        body = m_arena.allocate<BlockStmt>((std::vector<Stmt*>) { body, m_arena.allocate<ExprStmt>(increment) });
    }
    if (condition == nullptr) {
        condition = m_arena.allocate<Literal>(&Token::global_true_constant);
    }
    body = m_arena.allocate<WhileStmt>(condition, body);

    if (initializer != nullptr) {
        body = m_arena.allocate<BlockStmt>((std::vector<Stmt*>) { initializer, body });
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

        } while (match({ Token::COMMA }));
    }

    consume(Token::RIGHT_PAR, "Expected ) after function parameters");
    consume(Token::LEFT_SQUARE, "Expected [ befor function body");

    std::vector<Stmt*> body = block();
    return m_arena.allocate<FuncStmt>(name, parameters, body);
}

jl::Stmt* jl::Parser::return_statement()
{
    Token& return_token = previous();
    Expr* expr = nullptr;

    if (!check(Token::SEMI_COLON)) {
        expr = expression();
    }

    consume(Token::SEMI_COLON, "Expected ; after return");

    return m_arena.allocate<ReturnStmt>(return_token, expr);
}

jl::Stmt* jl::Parser::class_declaration()
{
    Token& name = consume(Token::IDENTIFIER, "Expected a class name");

    Variable* super_class = nullptr;
    if (match({ Token::COLON })) {
        consume(Token::IDENTIFIER, "Expected a super class name");
        super_class = m_arena.allocate<Variable>(previous());
    }

    consume(Token::LEFT_SQUARE, "Expected a [ before class body");

    std::vector<FuncStmt*> methods;
    while (!check(Token::RIGHT_SQUARE) && !is_at_end()) {
        methods.push_back(static_cast<FuncStmt*>(function("method")));
    }

    consume(Token::RIGHT_SQUARE, "Expected a ] after class body");
    return m_arena.allocate<ClassStmt>(name, super_class, methods);
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
