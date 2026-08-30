#include "Runner.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <string>
#include <string_view>
#include <unistd.h>

std::pair<uint32_t, std::string> run_via_pipes(std::string_view cmd)
{
    const auto out = popen(cmd.data(), "r");

    if (!out) {
        return { 1, "Failed to assemle source" };
    }

    std::array<char, 1024> buffer;
    std::string result;

    while (fgets(buffer.data(), buffer.size(), out) != nullptr) {
        result += buffer.data();
    }

    auto status = pclose(out);
    int exit_code = WEXITSTATUS(status);

    return { exit_code, result };
}

std::expected<uint32_t, std::string> jl::x86::run(std::string_view source)
{
    // Save the source to a file
    std::ofstream out_file("test.asm");
    out_file << source;
    out_file.close();

    // Run nasm
    const auto nasm_cmd = "nasm -f elf64 test.asm";
    if (auto [status, output] = run_via_pipes(nasm_cmd); status != 0) {
        return std::unexpected(output);
    }

    // Run linker
    const auto ld_cmd = "ld -o test test.o";
    if (auto [status, output] = run_via_pipes(ld_cmd); status != 0) {
        return std::unexpected(output);
    }

    // Run exe
    const auto exe_cmd = "./test 2>&1";
    auto [status, output] = run_via_pipes(exe_cmd);

    if (output != "") {
        return std::unexpected(output);
    } else {
        return status;
    }
}

std::string jl::x86::generate_executable_assembly_with_start_sym(std::string_view assembly,
    std::string_view function_to_call,
    std::initializer_list<std::string_view> arg_passing)
{
    std::string lines;
    std::for_each(arg_passing.begin(), arg_passing.end(), [&lines](auto&& line) {
        lines += line;
        lines += "\n";
    });

    return std::format(R"(
global _start

{}

_start:
    {}
    call {}
    mov rdi, rax
    mov rax, 0x3c
    syscall
    ret
    )",
        assembly,
        lines,
        function_to_call);
}
