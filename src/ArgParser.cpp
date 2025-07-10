#include "ArgParser.hpp"

#include <cstring>
#include <optional>
#include <print>
#include <string>
#include <unordered_set>

jl::ArgParser::ArgParser(int args, char const* argv[])
    : m_args(args)
    , m_argv(argv)
{
}

void jl::ArgParser::print_help()
{
    std::println("Usage: june [options] file...");
    std::println("Options:");
    std::println("-h\t--help\t\tTo print this help");
    std::println("-s\t--step-by-step\tTo run in step-by-step mode");
    std::println("-d\t--debug\t\tTo print the disassemby and other data");
}

std::optional<jl::ArgParser::Params> jl::ArgParser::parse()
{
    if (m_args <= 1) {
        std::println("No file to run!");
        std::println("Use june <args> <file> to compile and run a file");
        return std::nullopt;
    }

    std::optional<std::string> file_path;
    std::unordered_set<Options> options;
    bool incorrect_use = false;

    for (int i = 1; i < m_args; i++) {
        const auto arg = m_argv[i];
        if (strlen(arg) <= 0) {
            continue;
        }

        if (arg[0] == '-') {
            if (arg[1] == '-') {
                // std::println("long {}", &arg[2]);
                if (m_long_flags.contains(&arg[2])) {
                    options.insert(m_long_flags.at(&arg[2]));
                } else {
                    incorrect_use = true;
                    std::println("Unknown argument {}", &arg[2]);
                }
            } else {
                // std::println("short {}", &arg[1]);
                for (int i = 1; i < strlen(arg); i++) {
                    if (m_short_flags.contains(arg[i])) {
                        options.insert(m_short_flags.at(arg[i]));
                    } else {
                        incorrect_use = true;
                        std::println("Unknown argument {}", arg[i]);
                    }
                }
            }
        } else {
            file_path = arg;
            // std::println("file {}", arg);
        }
    }

    if (incorrect_use) {
        std::println("Use -h for help");
        return std::nullopt;
    }

    if (!file_path) {
        std::println("No file provided");
        std::println("Use -h for help");
        return std::nullopt;
    }

    Params params;
    params.file_name = *file_path;

    for (const auto opt : options) {
        switch (opt) {
        case HELP:
            print_help();
            return std::nullopt;
        case IR_DEBUG:
            params.debug = true;
            break;
        case STEP_BY_STEP:
            params.step_by_step = true;
            break;
        }
    }

    return params;
}