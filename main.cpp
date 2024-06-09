#include <iostream>

#include "Expr.hpp"

int main()
{
    std::cout << "Hello World\n";
    jl::Token::TokenType t1 = jl::Token::TokenType::INT;
    jl::Token::TokenType t2 = jl::Token::TokenType::FLOAT;
    jl::Token::TokenType t3 = jl::Token::TokenType::PLUS;
    std::string lx = "+";
    jl::Token::Value v1(123);
    jl::Token::Value v2(43.43);
    jl::Expr* l1 = new jl::Literal(v1, t1);
    jl::Expr* l2 = new jl::Literal(v2, t2);
    jl::Token tok = jl::Token(t3, lx, 1);
    jl::Binary b1(*l1, tok, *l2);

    jl::Unary u(tok, b1);

    jl::IExprVisitor* visitor = new jl::ParsetreePrinter();
    u.accept(*visitor);
}