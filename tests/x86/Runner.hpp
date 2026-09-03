#pragma once

#include <cstdint>
#include <expected>
#include <string>
#include <string_view>

namespace jl {
namespace x86 {

    std::expected<uint32_t, std::string> run(std::string_view source, std::string_view name = "test", std::string_view path = ".");

    std::string generate_executable_assembly_with_start_sym(std::string_view assembly,
        std::string_view function_to_call,
        std::initializer_list<std::string_view> arg_passing);

}
}
