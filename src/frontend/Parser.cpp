#include "Parser.hpp"

#include "ErrorHandler.hpp"
#include "Expr.hpp"
#include "Stmt.hpp"
#include "Token.hpp"
#include "Value.hpp"

jl::Parser::Parser(std::vector<Token>& tokens, std::string& file_name)
    : m_tokens(tokens)
    , m_file_name(file_name)
{
}

jl::Parser::~Parser()
{
    for (auto ref : m_allocated_refs) {
        delete ref;
    }
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

jl::Expr* jl::Parser::parse_list()
{
    std::vector<Expr*> list;

    while (!is_at_end() && peek().get_tokentype() != Token::RIGHT_BRACE) {
        Expr* expr = or_expr();
        list.push_back(expr);
        if (peek().get_tokentype() != Token::RIGHT_BRACE) {
            consume(Token::COMMA, "Lists hould be comma seperated");
        }
    }

    consume(Token::RIGHT_BRACE, "Lists should end with '}'");
    Expr* jlist = new JList(std::move(list));
    m_allocated_refs.push_back(jlist);
    return jlist;
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
        m_allocated_refs.push_back(expr);
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
        m_allocated_refs.push_back(expr);
    }
    return expr;
}

jl::Expr* jl::Parser::term()
{
    Expr* expr = factor();

    while (match({ Token::MINUS, Token::PLUS, Token::PERCENT })) {
        Token& oper = previous();
        Expr* right = factor();
        expr = new Binary(expr, &oper, right);
        m_allocated_refs.push_back(expr);
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
        m_allocated_refs.push_back(expr);
    }
    return expr;
}

jl::Expr* jl::Parser::unary()
{
    if (match({ Token::BANG, Token::MINUS })) {
        Token& oper = previous();
        Expr* right = unary();
        Expr* unary_epxr = new Unary(&oper, right);
        m_allocated_refs.push_back(unary_epxr);
        return unary_epxr;
    }

    return call();
}

jl::Expr* jl::Parser::primary()
{
    if (match({ Token::INT, Token::FLOAT, Token::STRING, Token::FALSE, Token::TRUE, Token::NULL_, Token::CHAR })) {
        Value* value = previous().get_value();
        Expr* literal = new Literal(value); // Should copy or just take a reference(now using reference)
        m_allocated_refs.push_back(literal);
        return literal;
    }
    if (match({ Token::THIS })) {
        Expr* this_expr = new This(previous());
        m_allocated_refs.push_back(this_expr);
        return this_expr;
    }
    if (match({ Token::IDENTIFIER })) {
        Expr* var = new Variable(previous());
        m_allocated_refs.push_back(var);
        return var;
    }
    if (match({ Token::LEFT_PAR })) {
        Expr* expr = expression();
        consume(Token::RIGHT_PAR, "Expected ) after expression");
        Expr* grouping = new Grouping(expr);
        m_allocated_refs.push_back(grouping);
        return grouping;
    }
    if (match({ Token::SUPER })) {
        Token& keyword = previous();
        consume(Token::DOT, "Expected '.' after super");
        Token& method = consume(Token::IDENTIFIER, "Expect superclass method name");
        Expr* super = new Super(keyword, method);
        m_allocated_refs.push_back(super);
        return super;
    }
    if (match({ Token::LEFT_BRACE })) {
        return parse_list();
    }

    ErrorHandler::error(m_file_name, "parsing", "primary expression", peek().get_line(), std::string("Expected a expression here ").append(" but found ").append(peek().get_lexeme()).c_str(), 0);
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
            Expr* assign = new Assign(value, name);
            m_allocated_refs.push_back(assign);
            return assign;
        } else if (dynamic_cast<Get*>(expr)) {
            Get* get_expr = static_cast<Get*>(expr);
            Expr* set = new Set(get_expr->m_name, get_expr->m_object, value);
            m_allocated_refs.push_back(set);
            return set;
        } else if (dynamic_cast<IndexGet*>(expr)) {
            IndexGet* index_get = static_cast<IndexGet*>(expr);
            Expr* index_set = new IndexSet(index_get->m_jlist, index_get->m_index_expr, value, index_get->m_closing_bracket);
            m_allocated_refs.push_back(index_set);
            return index_set;
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
    } else if (match({ Token::PERCENT_EQUAL })) {
        return modify_and_assign(Token::PERCENT, expr);
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
        m_allocated_refs.push_back(expr);
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
        m_allocated_refs.push_back(expr);
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
            expr = new Get(name, expr);
            m_allocated_refs.push_back(expr);
        } else if (match({ Token::LEFT_SQUARE })) {
            Expr* index_expr = or_expr();
            Token& closing_bracket = consume(Token::RIGHT_SQUARE, "Expected closing ] after indexing");
            expr = new IndexGet(expr, index_expr, closing_bracket);
            m_allocated_refs.push_back(expr);
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
    Expr* call = new Call(callee, paren, arguments);
    m_allocated_refs.push_back(call);
    return call;
}

jl::Expr* jl::Parser::modify_and_assign(Token::TokenType oper_type, Expr* expr)
{
    // TODO::Remove duplicate code
    Token& oper_equals = previous();
    Expr* value = or_expr();

    if (dynamic_cast<Variable*>(expr)) {
        Token& name = static_cast<Variable*>(expr)->m_name;
        Token* oper_token = new Token(oper_type, previous().get_lexeme(), previous().get_line());
        m_allocated_refs.push_back(oper_token);

        Binary* oper = new Binary(expr, oper_token, value);
        m_allocated_refs.push_back(oper);

        Expr* assign = new Assign(oper, name);
        m_allocated_refs.push_back(assign);
        return assign;
    } else if (dynamic_cast<Get*>(expr)) {
        Get* get_expr = static_cast<Get*>(expr);
        Token* oper_token = new Token(oper_type, previous().get_lexeme(), previous().get_line());
        m_allocated_refs.push_back(oper_token);

        Binary* oper = new Binary(expr, oper_token, value);
        m_allocated_refs.push_back(oper);

        Expr* set = new Set(get_expr->m_name, get_expr->m_object, oper);
        m_allocated_refs.push_back(set);
        return set;
    } else if (dynamic_cast<IndexGet*>(expr)) {
        IndexGet* index_get = static_cast<IndexGet*>(expr);
        Token* oper_token = new Token(oper_type, previous().get_lexeme(), previous().get_line());
        m_allocated_refs.push_back(oper_token);

        Binary* oper = new Binary(expr, oper_token, value);
        m_allocated_refs.push_back(oper);

        Expr* index_set = new IndexSet(index_get->m_jlist, index_get->m_index_expr, oper, index_get->m_closing_bracket);
        m_allocated_refs.push_back(index_set);
        return index_set;
    }

    ErrorHandler::error(m_file_name, "parsing", "add assignment", oper_equals.get_line(), "Invalid assignment target, expected a variable", 0);
    return expr;
}

// --------------------------------------------------------------------------------
// -------------------------------Statements---------------------------------------
// --------------------------------------------------------------------------------

jl::Stmt* jl::Parser::statement()
{
    if (match({ Token::LEFT_SQUARE })) {
        Stmt* block_stmt = new BlockStmt(block());
        m_allocated_refs.push_back(block_stmt);
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
            Stmt* empty = new EmptyStmt();
            m_allocated_refs.push_back(empty);
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

jl::Stmt* jl::Parser::print_statement()
{
    Expr* expr = expression();
    consume(Token::SEMI_COLON, "Expected ; after expression");
    Stmt* print_stmt = new PrintStmt(expr);
    m_allocated_refs.push_back(print_stmt);
    return print_stmt;
}

jl::Stmt* jl::Parser::expr_statement()
{
    Expr* expr = expression();
    consume(Token::SEMI_COLON, "Expected ; after expression");
    Stmt* expr_stmt = new ExprStmt(expr);
    m_allocated_refs.push_back(expr_stmt);
    return expr_stmt;
}

jl::Stmt* jl::Parser::var_declaration(bool for_each)
{
    Token& name = consume(Token::IDENTIFIER, "Expected a variable name");
    Expr* initializer = nullptr;
    Token* type_name = nullptr;

    // Variable with type declaration
    if (match({ Token::COLON })) {
        type_name = &consume(Token::IDENTIFIER, "Expected a data-type");
    }

    if (match({ Token::EQUAL })) {
        initializer = expression();
    }

    if (for_each) {
        if (!match({ Token::COLON, Token::SEMI_COLON })) {
            ErrorHandler::error(m_file_name, "parsing", "for each loop", name.get_line(), "Varible declaration should be followed `:` or `;` in a for loop", 0);
        }
        // consume(Token::COLON, "Expected : after variable declaration in for each loop");
    } else {
        consume(Token::SEMI_COLON, "Expected ; after variable declaration");
    }

    Stmt* var = new VarStmt(name, initializer, type_name);
    m_allocated_refs.push_back(var);
    return var;
}

jl::Stmt* jl::Parser::if_stmt()
{
    consume(Token::LEFT_PAR, "Expected ( after if keyword");
    Expr* condition = expression();
    consume(Token::RIGHT_PAR, "Expected ) after onditions in a if block");
    Stmt* then_branch = statement();

    Stmt* else_branch = nullptr;

    if (match({ Token::ELSE })) {
        else_branch = statement();
    }

    Stmt* if_stmt = new IfStmt(condition, then_branch, else_branch);
    m_allocated_refs.push_back(if_stmt);
    return if_stmt;
}

jl::Stmt* jl::Parser::while_statement()
{
    consume(Token::LEFT_PAR, "Expected ( after while keyword");
    Expr* condition = expression();
    consume(Token::RIGHT_PAR, "Expected ) after onditions in a while block");
    Stmt* body = statement();
    Stmt* while_stmt = new WhileStmt(condition, body);
    m_allocated_refs.push_back(while_stmt);
    return while_stmt;
}

jl::Stmt* jl::Parser::for_statement()
{
    consume(Token::LEFT_PAR, "Expected ( after for keyword");
    Stmt* initializer;
    bool declared_var = false;

    if (match({ Token::SEMI_COLON })) {
        initializer = nullptr;
    } else if (match({ Token::VAR })) {
        initializer = var_declaration(true);
        declared_var = true;
    } else {
        initializer = expr_statement();
    }

    // For each loop
    if (previous().get_tokentype() == Token::COLON) {
        if (!declared_var) {
            ErrorHandler::error(m_file_name, "parsing", "for each loop", previous().get_line(), "Varible declaration should precede `:` in a for each loop", 0);
        }
        // Use call() for now, change to maybe or_expr if errors occur
        Expr* list_expr = call();
        consume(Token::RIGHT_PAR, "Expected ) after all loop clauses");
        Stmt* body = statement();
        Stmt* for_each = new ForEachStmt(static_cast<VarStmt*>(initializer), list_expr, body);
        m_allocated_refs.push_back(for_each);
        return for_each;
    } else { // Normal For loop
        Expr* condition = nullptr;
        if (!check(Token::SEMI_COLON)) {
            condition = expression();
        }
        consume(Token::SEMI_COLON, "Expected ; after loop condition");

        Expr* increment = nullptr;
        if (!check(Token::SEMI_COLON)) {
            increment = expression();
        }
        consume(Token::RIGHT_PAR, "Expected ) after all loop clauses");

        Stmt* body = statement();

        if (increment != nullptr) {
            Stmt* expr_stmt = new ExprStmt(increment);
            body = new BlockStmt(std::vector<Stmt*> { body, expr_stmt });
            m_allocated_refs.push_back(body);
            m_allocated_refs.push_back(expr_stmt);
        }
        if (condition == nullptr) {
            condition = new Literal(&Token::global_true_constant);
            m_allocated_refs.push_back(condition);
        }
        body = new WhileStmt(condition, body);
        m_allocated_refs.push_back(body);

        if (initializer != nullptr) {
            body = new BlockStmt(std::vector<Stmt*> { initializer, body });
            m_allocated_refs.push_back(body);
        }

        return body;
    }
}

jl::Stmt* jl::Parser::function(const char* kind)
{
    FuncStmt* func = function_declaration();

    consume(Token::LEFT_SQUARE, "Expected [ before function body");
    std::vector<Stmt*> body = block();

    func->m_body = body;
    func->is_extern = false;

    return func;
}

jl::Stmt* jl::Parser::return_statement()
{
    Token& return_token = previous();
    Expr* expr = nullptr;

    if (!check(Token::SEMI_COLON)) {
        expr = expression();
    }

    consume(Token::SEMI_COLON, "Expected ; after return");
    Stmt* return_stmt = new ReturnStmt(return_token, expr);
    m_allocated_refs.push_back(return_stmt);
    return return_stmt;
}

jl::Stmt* jl::Parser::class_declaration()
{
    Token& name = consume(Token::IDENTIFIER, "Expected a class name");

    Variable* super_class = nullptr;
    if (match({ Token::COLON })) {
        consume(Token::IDENTIFIER, "Expected a super class name");
        super_class = new Variable(previous());
        m_allocated_refs.push_back(super_class);
    }

    consume(Token::LEFT_SQUARE, "Expected a [ before class body");

    std::vector<FuncStmt*> methods;
    while (!check(Token::RIGHT_SQUARE) && !is_at_end()) {
        methods.push_back(static_cast<FuncStmt*>(function("method")));
    }

    consume(Token::RIGHT_SQUARE, "Expected a ] after class body");
    Stmt* class_stmt = new ClassStmt(name, super_class, methods);
    m_allocated_refs.push_back(class_stmt);
    return class_stmt;
}

jl::Stmt* jl::Parser::break_statement()
{
    Token& break_token = previous();

    consume(Token::SEMI_COLON, "Expected ; after break");
    Stmt* break_stmt = new BreakStmt(break_token);
    m_allocated_refs.push_back(break_stmt);
    return break_stmt;
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

jl::Stmt* jl::Parser::extern_declaration()
{
    Token& extern_token = previous();
    Token& symbol_name = consume(Token::STRING, "Expected symbol name as str after `extern`");
    consume(Token::AS, "Expected `as` after symbol name");
    FuncStmt* june_func = function_declaration();
    consume(Token::SEMI_COLON, "Expected ; after extern declaration");

    Stmt* extern_stmt = new ExternStmt(extern_token, symbol_name, june_func);
    m_allocated_refs.push_back(extern_stmt);
    return extern_stmt;
}

jl::FuncStmt* jl::Parser::function_declaration()
{
    Token& name = consume(Token::IDENTIFIER, "Expeced a function name here");

    consume(Token::LEFT_PAR, "Expected ( after fun name");
    std::vector<Token*> parameters;
    std::vector<Token*> data_types;

    if (!check(Token::RIGHT_PAR)) {
        do {
            if (parameters.size() >= 255) {
                ErrorHandler::error(m_file_name, "parsing", "function call", peek().get_line(), "Cannot have more than 255 parameters for a function", 0);
            }

            Token& param = consume(Token::IDENTIFIER, "Expected parameter name here");
            consume(Token::COLON, "Expected a colon after parameter name");
            Token& data_type = consume(Token::IDENTIFIER, "Expected data type here after :");

            parameters.push_back(&param);
            data_types.push_back(&data_type);

        } while (match({ Token::COMMA }));
    }

    consume(Token::RIGHT_PAR, "Expected ) after function parameters");

    Token* return_type = nullptr;
    if (match({ Token::COLON })) {
        return_type = &consume(Token::IDENTIFIER, "Expected return data type here after :");
    }

    FuncStmt* func = new FuncStmt(name, parameters, data_types, return_type);
    m_allocated_refs.push_back(func);
    return func;
}