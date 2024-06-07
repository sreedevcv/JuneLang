#include <iostream>

#include "Lexer.hpp"

int main()
{
    std::cout << "Hello World\n";

    std::string file_name = "../test/lexer_test.jun";
    jl::Lexer lexer(file_name);
    lexer.scan();
}