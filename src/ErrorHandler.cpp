#include "ErrorHandler.hpp"

#include <iostream>

bool jl::ErrorHandler::error_occured = false;
int jl::ErrorHandler::error_count = 0;

void jl::ErrorHandler::error(std::string& file_name, int line, const char* msg)
{
    error(file_name, line, msg, 1);
}

void jl::ErrorHandler::error(std::string& file_name, int line, const char* msg, int char_loc)
{
    std::cout << "[ERROR]::" << file_name << ':' << line;
    if (char_loc != 0)
        std::cout << ':' << char_loc;
    std::cout << " " << msg << std::endl;

    error_occured = true;
    error_count += 1;
}

bool jl::ErrorHandler::has_error()
{
    return error_occured;
}

int jl::ErrorHandler::get_error_count()
{
    return error_count;
}
