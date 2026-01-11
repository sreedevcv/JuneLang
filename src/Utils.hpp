#pragma once

#include <cstdlib>
#include <print>
#include <stacktrace>

inline void unimplemented(const char* msg = "")
{
    std::println("[Unimplemented]({})\n{}", msg, std::stacktrace::current());
    std::exit(1);
}

namespace jl {
        using reg_type = uint64_t;
}