#pragma once

#include <optional>
#include <string>
#include <unordered_map>

namespace jl {

class ArgParser {
public:
    ArgParser(int args, char const* argv[]);

    struct Params {
        std::string file_name;
        bool step_by_step;
        bool debug;
    };

    std::optional<Params> parse();

private:
    enum Options {
        HELP,
        IR_DEBUG,
        STEP_BY_STEP,
    };

    std::unordered_map<char, Options> m_short_flags {
        { 's', STEP_BY_STEP },
        { 'd', IR_DEBUG },
        { 'h', HELP },
    };

    std::unordered_map<std::string, Options> m_long_flags {
        { "step-by-step", STEP_BY_STEP },
        { "debug", IR_DEBUG },
        { "help", HELP },
    };

    int m_args;
    char const** m_argv;

    void print_help();
};

}