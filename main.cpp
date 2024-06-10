#include <iostream>

#include "ErrorHandler.hpp"
#include "Expr.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"

int main()
{
    std::cout << "Hello World\n";
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