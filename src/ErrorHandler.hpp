#pragma once

#include <string>
#include "StreamHandler.hpp"

namespace jl {
class ErrorHandler {
public:
    static void error(std::string& file_name, int line, const char* msg);
    static void error(std::string& file_name, int line, const char* msg, int char_loc);
    static void error(std::string& file_name, const char* where, const char* when, int line, const char* msg, int char_loc);

    static bool has_error();
    static int get_error_count();
    static StreamHandler m_stream;

    static void clear_errors();
    static std::stringstream& get_string_stream();

private:
    static int error_count;
};
} // namespace jl