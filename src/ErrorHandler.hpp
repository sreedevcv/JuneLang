#pragma once

#include <string>

namespace jl {
class ErrorHandler {
public:
    static void error(std::string& file_name, int line, const char* msg);
    static void error(std::string& file_name, int line, const char* msg, int char_loc);

    static bool has_error();
    static int get_error_count();

private:
    static bool error_occured;
    static int error_count;
};
} // namespace jl