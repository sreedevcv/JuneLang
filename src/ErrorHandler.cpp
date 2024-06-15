#include "ErrorHandler.hpp"

#include <iostream>
#include <cstring>

int jl::ErrorHandler::error_count = 0;

void jl::ErrorHandler::error(std::string& file_name, int line, const char* msg)
{
    error(file_name, line, msg, 0);
}

void jl::ErrorHandler::error(std::string& file_name, int line, const char* msg, int char_loc)
{
    error(file_name, "", "", line, msg, char_loc);
}

void jl::ErrorHandler::error(std::string& file_name, const char* where, const char* when, int line, const char* msg, int char_loc)
{
    std::cout << "[ERROR] " << file_name << ':' << line;
    if (char_loc != 0) {
        std::cout << ' at ' << char_loc;
    }
    if (std::strcmp(where, "") != 0) {
        std::cout << " during [" << where << "]";
    }
    if (std::strcmp(when, "") != 0) {
        std::cout << " when handling [" << when << "]";
    }
    std::cout << " ::\n\t" << msg << std::endl;

    error_count += 1;

}

bool jl::ErrorHandler::has_error()
{
    return error_count > 0;
}

int jl::ErrorHandler::get_error_count()
{
    return error_count;
}
