#include "Runner.hpp"

#include <array>
#include <cstdio>
#include <fstream>
#include <string>
#include <string_view>
#include <unistd.h>

std::expected<uint32_t, std::string> run_via_pipes(std::string_view cmd)
{
    const auto out = popen(cmd.data(), "r");

    if (!out) {
        return std::unexpected("Failed to assemle source");
    }

    std::array<char, 1024> buffer;
    std::string result;

    while (fgets(buffer.data(), buffer.size(), out) != nullptr) {
        result += buffer.data();
    }

    int status = pclose(out);

    if (status != 0) {
        return std::unexpected(result);
    }

    return status;
}

std::expected<uint32_t, std::string> jl::x86::run(std::string_view source)
{
    // Save the source to a file
    std::ofstream out_file("test.asm");
    out_file << source;
    out_file.close();

    // Run nasm
    const auto nasm_cmd = "nams -f elf64 test.asm";
    if (auto error = run_via_pipes(nasm_cmd)) {
        return error;
    }

    // Run linker
    const auto ld_cmd = "ld -o test test.asm";
    if (auto error = run_via_pipes(ld_cmd)) {
        return error;
    }

    // Run exe
    const auto exe_cmd = "./test";
    return run_via_pipes(exe_cmd);
}
