#include <iostream>

#include "Lexer.hpp"

int main()
{
    std::cout << "Hello World\n";

    std::string file_name = "../tests/scripts/lexer_test_2.jun";
    jl::Lexer lexer(file_name);
    lexer.scan();

    std::cout << lexer.get_tokens().size() << "\n";
}