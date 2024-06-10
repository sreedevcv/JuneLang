#include <iostream>

#include "ErrorHandler.hpp"
#include "Expr.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"

int main()
{
    std::cout << "Hello World\n";
    // jl::Token::TokenType t1 = jl::Token::TokenType::INT;
    // jl::Token::TokenType t2 = jl::Token::TokenType::FLOAT;
    // jl::Token::TokenType t3 = jl::Token::TokenType::PLUS;
    // std::string lx = "+";
    // jl::Unary u(new jl::Token(t3, lx, 1),
    //     new jl::Binary(
    //         new jl::Literal(new jl::Token::Value(123), t1),
    //         new jl::Token(t3, lx, 1),
    //         new jl::Literal(new jl::Token::Value(12.5), t2)));
    // jl::IExprVisitor* visitor = new jl::ParsetreePrinter();
    // u.accept(*visitor, visitor->get_context());
    // std::string* context = (std::string*)visitor->get_context();
    // std::cout << *context << "\n";
    std::string file = "../tests/scripts/parser_test_1.jun";
    jl::Lexer lexer(file);

    if (!jl::ErrorHandler::has_error()) {
        lexer.scan();
        auto tokens = lexer.get_tokens();
        jl::Parser parser(tokens);
        jl::Expr* e = parser.parse();
        if (e == nullptr) return 0;

        jl::IExprVisitor* visitor = new jl::ParsetreePrinter();
        e->accept(*visitor, visitor->get_context());
        std::string* context = (std::string*)visitor->get_context();
        std::cout << *context << "\n";
    }
}