#include "ErrorHandler.hpp"

#include <iostream>
#include <cstring>

int jl::ErrorHandler::error_count = 0;
jl::StreamHandler jl::ErrorHandler::m_stream = jl::StreamHandler(); 

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
    m_stream << "[ERROR] " << file_name << ':' << line;
    if (char_loc != 0) {
        m_stream << " at " << char_loc;
    }
    if (std::strcmp(where, "") != 0) {
        m_stream << " during [" << where << "]";
    }
    if (std::strcmp(when, "") != 0) {
        m_stream << " when handling [" << when << "]";
    }
    m_stream << " ::\n\t" << msg << std::endl;

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

void jl::ErrorHandler::reset()
{
    error_count = 0;
}

void jl::ErrorHandler::clear_errors()
{
    error_count = 0;
}

std::stringstream& jl::ErrorHandler::get_string_stream()
{
    return m_stream.get_string_stream();
}
