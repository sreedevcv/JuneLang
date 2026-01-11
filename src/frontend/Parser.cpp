#include "Parser.hpp"

#include "ErrorHandler.hpp"
#include "Expr.hpp"
#include "Stmt.hpp"
#include "Token.hpp"
#include "TypeInfo.hpp"
#include "Value.hpp"

#include <memory>
#include <optional>
#include <utility>

jl::Parser::Parser(std::vector<Token>& tokens, std::string& file_name)
    : m_tokens(tokens)
    , m_file_name(file_name)
{
}

jl::Parser::~Parser()
{
}

std::unique_ptr<jl::Expr> jl::Parser::parse()
{
    try {
        return expression();
    } catch (const char* e) {
        return nullptr;
    }
}

std::vector<std::unique_ptr<jl::Stmt>> jl::Parser::parseStatements()
{
    std::vector<std::unique_ptr<Stmt>> statements;
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
        case Token::BREAK:
            return;
        default:
            break;
        }

        advance();
    }
}

bool jl::Parser::match(std::initializer_list<Token::TokenType>&& types)
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

std::unique_ptr<jl::Expr> jl::Parser::parse_list()
{
    std::vector<std::unique_ptr<Expr>> list;

    while (!is_at_end() && peek().get_tokentype() != Token::RIGHT_BRACE) {
        // Expr* expr = or_expr();
        list.push_back(or_expr());
        if (peek().get_tokentype() != Token::RIGHT_BRACE) {
            consume(Token::COMMA, "Lists hould be comma seperated");
        }
    }

    auto right_brace = consume(Token::RIGHT_BRACE, "Lists should end with '}'");
    auto jlist = std::make_unique<JList>(std::move(list), std::move(right_brace));
    return jlist;
}

// --------------------------------------------------------------------------------
// -------------------------------Expressions--------------------------------------
// --------------------------------------------------------------------------------

std::unique_ptr<jl::Expr> jl::Parser::expression()
{
    return assignment();
}

std::unique_ptr<jl::Expr> jl::Parser::equality()
{
    auto expr = comparison();

    while (match({ Token::BANG_EQUAL, Token::EQUAL_EQUAL })) {
        Token& oper = previous();
        auto right = comparison();
        expr = std::make_unique<Binary>(std::move(expr), oper, std::move(right));
    }

    return expr;
}

std::unique_ptr<jl::Expr> jl::Parser::comparison()
{
    auto expr = term();

    while (match({ Token::GREATER, Token::GREATER_EQUAL, Token::LESS, Token::LESS_EQUAL })) {
        Token& oper = previous();
        auto right = term();
        expr = std::make_unique<Binary>(std::move(expr), oper, std::move(right));
    }
    return expr;
}

std::unique_ptr<jl::Expr> jl::Parser::term()
{
    auto expr = factor();

    while (match({ Token::MINUS, Token::PLUS, Token::BIT_AND, Token::BIT_OR, Token::BIT_XOR })) {
        Token& oper = previous();
        auto right = factor();
        expr = std::make_unique<Binary>(std::move(expr), oper, std::move(right));
    }
    return expr;
}

std::unique_ptr<jl::Expr> jl::Parser::factor()
{
    auto expr = type_cast();

    while (match({ Token::SLASH, Token::STAR, Token::PERCENT })) {
        Token& oper = previous();
        auto right = type_cast();
        expr = std::make_unique<Binary>(std::move(expr), oper, std::move(right));
    }
    return expr;
}

std::unique_ptr<jl::Expr> jl::Parser::type_cast()
{
    auto expr = unary();

    while (match({ Token::AS })) {
        Token& oper = previous();
        auto right = parse_type_info();

        if (!right) {
            ErrorHandler::error(
                m_file_name,
                "parsing",
                "type cast expr",
                oper.get_line(),
                "Invalid data type",
                0);
            right = TypeInfo {};
        }

        expr = std::make_unique<TypeCast>(std::move(expr), *right);
    }
    return expr;
}

std::unique_ptr<jl::Expr> jl::Parser::unary()
{
    if (match({ Token::BANG, Token::MINUS, Token::BIT_NOT })) {
        Token& oper = previous();
        auto right = unary();
        auto unary_epxr = std::make_unique<Unary>(oper, std::move(right));
        return unary_epxr;
    }

    return call();
}

std::unique_ptr<jl::Expr> jl::Parser::primary()
{
    if (match({ Token::INT, Token::FLOAT, Token::STRING, Token::FALSE, Token::TRUE, Token::NULL_, Token::CHAR })) {
        Value value = previous().get_value();
        auto literal = std::make_unique<Literal>(value);
        return literal;
    }
    if (match({ Token::THIS })) {
        auto this_expr = std::make_unique<This>(previous());
        return this_expr;
    }
    if (match({ Token::IDENTIFIER })) {
        auto var = std::make_unique<Variable>(previous());
        return var;
    }
    if (match({ Token::LEFT_PAR })) {
        auto expr = expression();
        consume(Token::RIGHT_PAR, "Expected ) after expression");
        auto grouping = std::make_unique<Grouping>(std::move(expr));
        return grouping;
    }
    if (match({ Token::SUPER })) {
        Token& keyword = previous();
        consume(Token::DOT, "Expected '.' after super");
        Token& method = consume(Token::IDENTIFIER, "Expect superclass method name");
        auto super = std::make_unique<Super>(keyword, method);
        return super;
    }
    if (match({ Token::LEFT_BRACE })) {
        return parse_list();
    }

    ErrorHandler::error(
        m_file_name,
        "parsing",
        "primary expression",
        peek().get_line(),
        std::string("Expected a expression here ").append(" but found ").append(peek().get_lexeme()).c_str(),
        0);
    throw "parse-exception";
}

std::unique_ptr<jl::Expr> jl::Parser::assignment()
{
    auto expr = or_expr();

    if (match({ Token::EQUAL })) {
        Token& equals = previous();
        auto value = assignment();

        if (dynamic_cast<Variable*>(expr.get())) {
            Token& name = static_cast<Variable*>(expr.get())->m_name;
            auto assign = std::make_unique<Assign>(std::move(value), name);
            return assign;
        } else if (dynamic_cast<Get*>(expr.get())) {
            Get* get_expr = static_cast<Get*>(expr.get());
            auto set = std::make_unique<Set>(get_expr->m_name, std::move(get_expr->m_object), std::move(value));
            return set;
        } else if (dynamic_cast<IndexGet*>(expr.get())) {
            IndexGet* index_get = static_cast<IndexGet*>(expr.get());
            auto index_set = std::make_unique<IndexSet>(
                std::move(index_get->m_jlist),
                std::move(index_get->m_index_expr),
                std::move(value),
                index_get->m_closing_bracket);
            return index_set;
        }

        ErrorHandler::error(
            m_file_name,
            "parsing",
            "assignment",
            equals.get_line(),
            "Invalid assignment target, expected a variable",
            0);

    } else if (match({ Token::PLUS_EQUAL })) {
        return modify_and_assign(Token::PLUS, std::move(expr));
    } else if (match({ Token::MINUS_EQUAL })) {
        return modify_and_assign(Token::MINUS, std::move(expr));
    } else if (match({ Token::STAR_EQUAL })) {
        return modify_and_assign(Token::STAR, std::move(expr));
    } else if (match({ Token::SLASH_EQUAL })) {
        return modify_and_assign(Token::SLASH, std::move(expr));
    } else if (match({ Token::PERCENT_EQUAL })) {
        return modify_and_assign(Token::PERCENT, std::move(expr));
    }

    return expr;
}

std::unique_ptr<jl::Expr> jl::Parser::or_expr()
{
    auto expr = and_expr();

    while (match({ Token::OR })) {
        Token& oper = previous();
        auto right = and_expr();
        expr = std::make_unique<Logical>(std::move(expr), oper, std::move(right));
    }

    return expr;
}

std::unique_ptr<jl::Expr> jl::Parser::and_expr()
{
    auto expr = equality();

    while (match({ Token::AND })) {
        Token& oper = previous();
        auto right = equality();
        expr = std::make_unique<Logical>(std::move(expr), oper, std::move(right));
    }

    return expr;
}

std::unique_ptr<jl::Expr> jl::Parser::call()
{
    auto expr = primary();

    while (true) {
        if (match({ Token::LEFT_PAR })) {
            expr = finish_call(std::move(expr));
        } else if (match({ Token::DOT })) {
            Token& name = consume(Token::IDENTIFIER, "Expected property name after '.'");
            expr = std::make_unique<Get>(name, std::move(expr));
        } else if (match({ Token::LEFT_SQUARE })) {
            auto index_expr = or_expr();
            Token& closing_bracket = consume(Token::RIGHT_SQUARE, "Expected closing ] after indexing");
            expr = std::make_unique<IndexGet>(std::move(expr), std::move(index_expr), closing_bracket);
        } else {
            break;
        }
    }

    return expr;
}

std::unique_ptr<jl::Expr> jl::Parser::finish_call(std::unique_ptr<Expr> callee)
{
    std::vector<std::unique_ptr<Expr>> arguments;

    if (!check(Token::RIGHT_PAR)) {
        do {
            if (arguments.size() > 255) {
                ErrorHandler::error(
                    m_file_name,
                    "parsing",
                    "function call",
                    peek().get_line(),
                    "Cannot have more than 255 args for a call",
                    0);
            }
            arguments.push_back(expression());
        } while (match({ Token::COMMA }));
    }

    Token& paren = consume(Token::RIGHT_PAR, "Expected ) after arguments");
    auto call = std::make_unique<Call>(std::move(callee), paren, std::move(arguments));
    return call;
}

std::unique_ptr<jl::Expr> jl::Parser::modify_and_assign(Token::TokenType oper_type, std::unique_ptr<Expr> expr)
{
    // TODO::Remove duplicate code
    Token& oper_equals = previous();
    auto value = or_expr();

    if (dynamic_cast<Variable*>(expr.get())) {
        Token& name = static_cast<Variable*>(expr.get())->m_name;
        Token oper_token = Token(oper_type, m_tokens[m_current - 2].get_lexeme(), m_tokens[m_current - 2].get_line());
        auto oper = std::make_unique<Binary>(std::move(expr), oper_token, std::move(value));
        auto assign = std::make_unique<Assign>(std::move(oper), name);
        return assign;
    } else if (dynamic_cast<Get*>(expr.get())) {
        Get* get_expr = static_cast<Get*>(expr.get());
        Token oper_token = Token(oper_type, previous().get_lexeme(), previous().get_line());
        auto oper = std::make_unique<Binary>(std::move(expr), oper_token, std::move(value));
        auto set = std::make_unique<Set>(get_expr->m_name, std::move(get_expr->m_object), std::move(oper));
        return set;
    } else if (dynamic_cast<IndexGet*>(expr.get())) {
        IndexGet* index_get = static_cast<IndexGet*>(expr.get());
        Token oper_token = Token(oper_type, previous().get_lexeme(), previous().get_line());
        auto oper = std::make_unique<Binary>(std::move(expr), oper_token, std::move(value));
        auto index_set = std::make_unique<IndexSet>(
            std::move(index_get->m_jlist),
            std::move(index_get->m_index_expr),
            std::move(oper),
            index_get->m_closing_bracket);
        return index_set;
    }

    ErrorHandler::error(
        m_file_name,
        "parsing",
        "add assignment",
        oper_equals.get_line(),
        "Invalid assignment target, expected a variable",
        0);
    return expr;
}

// --------------------------------------------------------------------------------
// -------------------------------Statements---------------------------------------
// --------------------------------------------------------------------------------

std::unique_ptr<jl::Stmt> jl::Parser::statement()
{
    if (match({ Token::LEFT_SQUARE })) {
        auto block_stmt = std::make_unique<BlockStmt>(block());
        return block_stmt;
    }
    if (match({ Token::PRINT })) {
        return print_statement();
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
    if (match({ Token::BREAK })) {
        return break_statement();
    }

    return expr_statement();
}

std::unique_ptr<jl::Stmt> jl::Parser::declaration()
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
            auto empty = std::make_unique<EmptyStmt>();
            return empty;
        }
        if (match({ Token::EXTERN })) {
            return extern_declaration();
        }
        return statement();
    } catch (const char* e) {
        synchronize();
        return nullptr;
    }
}

std::unique_ptr<jl::Stmt> jl::Parser::print_statement()
{
    auto expr = expression();
    auto token = consume(Token::SEMI_COLON, "Expected ; after expression");
    auto print_stmt = std::make_unique<PrintStmt>(std::move(expr), std::move(token));
    return print_stmt;
}

std::unique_ptr<jl::Stmt> jl::Parser::expr_statement()
{
    auto expr = expression();
    consume(Token::SEMI_COLON, "Expected ; after expression");
    auto expr_stmt = std::make_unique<ExprStmt>(std::move(expr));
    return expr_stmt;
}

std::unique_ptr<jl::Stmt> jl::Parser::var_declaration(bool for_each)
{
    Token& name = consume(Token::IDENTIFIER, "Expected a variable name");
    std::optional<std::unique_ptr<Expr>> initializer = std::nullopt;
    Token* type_name = nullptr;

    // Variable with type declaration
    auto type_info = match({ Token::COLON })
        ? parse_type_info()
        : std::nullopt;

    if (match({ Token::EQUAL })) {
        initializer = expression();
    }

    if (for_each) {
        if (!match({ Token::COLON, Token::SEMI_COLON })) {
            ErrorHandler::error(
                m_file_name,
                "parsing",
                "for each loop",
                name.get_line(),
                "Varible declaration should be followed `:` or `;` in a for loop",
                0);
        }
        // consume(Token::COLON, "Expected : after variable declaration in for each loop");
    } else {
        consume(Token::SEMI_COLON, "Expected ; after variable declaration");
    }

    auto var = std::make_unique<VarStmt>(name, std::move(initializer), std::move(type_info));
    return var;
}

std::unique_ptr<jl::Stmt> jl::Parser::if_stmt()
{
    auto left_par = consume(Token::LEFT_PAR, "Expected ( after if keyword");
    auto condition = expression();
    consume(Token::RIGHT_PAR, "Expected ) after onditions in a if block");
    auto then_branch = statement();

    std::optional<std::unique_ptr<Stmt>> else_branch = std::nullopt;

    if (match({ Token::ELSE })) {
        else_branch = statement();
    }

    auto if_stmt = std::make_unique<IfStmt>(
        std::move(condition), std::move(then_branch),
        std::move(else_branch), std::move(left_par));
    return if_stmt;
}

std::unique_ptr<jl::Stmt> jl::Parser::while_statement()
{
    auto left_par = consume(Token::LEFT_PAR, "Expected ( after while keyword");
    auto condition = expression();
    consume(Token::RIGHT_PAR, "Expected ) after onditions in a while block");
    auto body = statement();
    auto while_stmt = std::make_unique<WhileStmt>(std::move(condition), std::move(body), std::move(left_par));
    return while_stmt;
}

std::unique_ptr<jl::Stmt> jl::Parser::for_statement()
{
    auto left_par = consume(Token::LEFT_PAR, "Expected ( after for keyword");
    std::optional<std::unique_ptr<Stmt>> initializer = std::nullopt;
    bool declared_var = false;

    if (match({ Token::SEMI_COLON })) {
        initializer = std::nullopt;
    } else if (match({ Token::VAR })) {
        initializer = var_declaration(true);
        declared_var = true;
    } else {
        initializer = expr_statement();
    }

    // For each loop
    if (previous().get_tokentype() == Token::COLON) {
        if (!declared_var) {
            ErrorHandler::error(
                m_file_name,
                "parsing",
                "for each loop",
                previous().get_line(),
                "Varible declaration should precede `:` in a for each loop",
                0);
        }
        // Use call() for now, change to maybe or_expr if errors occur
        auto list_expr = call();
        consume(Token::RIGHT_PAR, "Expected ) after all loop clauses");
        auto body = statement();
        auto for_each = std::make_unique<ForEachStmt>(std::move(*initializer), std::move(list_expr), std::move(body));
        return for_each;
    } else { // Normal For loop
        std::optional<std::unique_ptr<Expr>> condition = std::nullopt;
        if (!check(Token::SEMI_COLON)) {
            condition = expression();
        }
        consume(Token::SEMI_COLON, "Expected ; after loop condition");

        std::optional<std::unique_ptr<Expr>> increment = std::nullopt;
        if (!check(Token::SEMI_COLON)) {
            increment = expression();
        }
        consume(Token::RIGHT_PAR, "Expected ) after all loop clauses");

        auto body = statement();

        if (increment) {
            auto expr_stmt = std::make_unique<ExprStmt>(std::move(*increment));
            std::vector<std::unique_ptr<Stmt>> stmts;
            stmts.push_back(std::move(body));
            stmts.push_back(std::move(expr_stmt));
            body = std::make_unique<BlockStmt>(std::move(stmts));
        }
        if (!condition) {
            auto true_val = Token::global_true_constant;
            condition = std::make_unique<Literal>(true_val);
        }

        body = std::make_unique<WhileStmt>(std::move(*condition), std::move(body), std::move(left_par));

        if (initializer) {
            std::vector<std::unique_ptr<Stmt>> stmts;
            stmts.push_back(std::move(*initializer));
            stmts.push_back(std::move(body));
            body = std::make_unique<BlockStmt>(std::move(stmts));
        }

        return body;
    }
}

std::unique_ptr<jl::Stmt> jl::Parser::function(const char* kind)
{
    auto func = function_declaration();

    consume(Token::LEFT_SQUARE, "Expected [ before function body");

    func->m_body = block();
    func->is_extern = false;

    return func;
}

std::unique_ptr<jl::Stmt> jl::Parser::return_statement()
{
    Token& return_token = previous();
    std::optional<std::unique_ptr<Expr>> expr = std::nullopt;

    if (!check(Token::SEMI_COLON)) {
        expr = expression();
    }

    consume(Token::SEMI_COLON, "Expected ; after return");
    auto return_stmt = std::make_unique<ReturnStmt>(return_token, std::move(expr));
    return return_stmt;
}

std::unique_ptr<jl::Stmt> jl::Parser::class_declaration()
{
    // Token& name = consume(Token::IDENTIFIER, "Expected a class name");

    // Variable* super_class = nullptr;
    // if (match({ Token::COLON })) {
    //     consume(Token::IDENTIFIER, "Expected a super class name");
    //     super_class = std::make_unique<Variable>(previous());
    //     m_allocated_refs.push_back(super_class);
    // }

    // consume(Token::LEFT_SQUARE, "Expected a [ before class body");

    // std::vector<FuncStmt*> methods;
    // while (!check(Token::RIGHT_SQUARE) && !is_at_end()) {
    //     methods.push_back(static_cast<FuncStmt*>(function("method")));
    // }

    // consume(Token::RIGHT_SQUARE, "Expected a ] after class body");
    // Stmt* class_stmt = std::make_unique<ClassStmt>(name, super_class, methods);
    // m_allocated_refs.push_back(class_stmt);
    // return class_stmt;

    return nullptr;
}

std::unique_ptr<jl::Stmt> jl::Parser::break_statement()
{
    Token& break_token = previous();
    consume(Token::SEMI_COLON, "Expected ; after break");
    auto break_stmt = std::make_unique<BreakStmt>(break_token);
    return break_stmt;
}

std::vector<std::unique_ptr<jl::Stmt>> jl::Parser::block()
{
    std::vector<std::unique_ptr<Stmt>> statements;

    while (!check(Token::RIGHT_SQUARE) && !is_at_end()) {
        statements.push_back(declaration());
    }

    consume(Token::RIGHT_SQUARE, "Expected ] after block");
    return statements;
}

std::unique_ptr<jl::Stmt> jl::Parser::extern_declaration()
{
    Token& extern_token = previous();
    Token& symbol_name = consume(Token::STRING, "Expected symbol name as str after `extern`");
    consume(Token::AS, "Expected `as` after symbol name");
    auto june_func = function_declaration();
    consume(Token::SEMI_COLON, "Expected ; after extern declaration");

    auto extern_stmt = std::make_unique<ExternStmt>(extern_token, symbol_name, std::move(june_func));
    return extern_stmt;
}

std::unique_ptr<jl::FuncStmt> jl::Parser::function_declaration()
{
    Token& name = consume(Token::IDENTIFIER, "Expeced a function name here");

    consume(Token::LEFT_PAR, "Expected ( after fun name");
    std::vector<Token*> parameters;
    std::vector<TypeInfo> data_types;

    if (!check(Token::RIGHT_PAR)) {
        do {
            if (parameters.size() >= 255) {
                ErrorHandler::error(
                    m_file_name,
                    "parsing",
                    "function call",
                    peek().get_line(),
                    "Cannot have more than 255 parameters for a function",
                    0);
            }

            Token& param = consume(Token::IDENTIFIER, "Expected parameter name here");

            consume(Token::COLON, "Expected : after param name");
            auto type_info = parse_type_info();

            if (!type_info) {
                ErrorHandler::error(m_file_name, name.get_line(), "Expected type after param name");
                type_info = {};
            }

            parameters.push_back(&param);
            data_types.emplace_back(*type_info);
        } while (match({ Token::COMMA }));
    }

    consume(Token::RIGHT_PAR, "Expected ) after function parameters");

    std::optional<TypeInfo> return_type = std::nullopt;
    if (match({ Token::COLON })) {
        return_type = parse_type_info();
        if (!return_type) {
            ErrorHandler::error(m_file_name, name.get_line(), "Expected return data type here after :");
        }
    }

    auto func = std::make_unique<FuncStmt>(name, parameters, std::move(data_types), return_type);
    return func;
}

std::optional<jl::TypeInfo> jl::Parser::parse_type_info()
{
    const auto& next = peek();

    if (next.get_tokentype() == Token::IDENTIFIER) {
        auto& type_name = consume(Token::IDENTIFIER, "Expected a data-type");
        return TypeInfo { .name = type_name.get_lexeme(), .is_array = false };
    } else if (next.get_tokentype() == Token::LEFT_SQUARE) {
        consume(Token::LEFT_SQUARE, "Expected [");

        auto& type_name = consume(Token::IDENTIFIER, "Expected a data-type");

        if (match({ Token::SEMI_COLON })) {
            auto& array_size = consume(Token::INT, "Expected a non-negative array size");
            consume(Token::RIGHT_SQUARE, "Expected ] after list type");

            const auto size = std::get<int>(array_size.get_value());
            return TypeInfo {
                .name = type_name.get_lexeme(),
                .is_array = true,
                .size = size,
            };
        }

        consume(Token::RIGHT_SQUARE, "Expected ] after list type");
        return TypeInfo {
            .name = type_name.get_lexeme(),
            .is_array = true,
        };
    }

    return std::nullopt;
}
