#pragma once

#include <cstdlib>
#include <print>
#include <stacktrace>

inline void print_stacktrace_and_exit(const char* msg_typ, const char* msg)
{
    std::println("{} ({})\n{}", msg_typ, msg, std::stacktrace::current());
    std::exit(1);
}

inline void unimplemented(const char* msg = "")
{
    print_stacktrace_and_exit("[Unimplemented]", msg);
}

template <typename T>
T unreachable(T&& t, const char* msg = "")
{
    print_stacktrace_and_exit("[Unreachable]", msg);
    return t;
}

inline void unreachable(const char* msg = "")
{
    print_stacktrace_and_exit("[Unreachable]", msg);
}

namespace jl {
using reg_type = uint64_t;
}