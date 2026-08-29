#pragma once

#include <cstdint>
#include <expected>
#include <string>
#include <string_view>

namespace jl {
namespace x86 {

    std::expected<uint32_t, std::string> run(std::string_view source);

}
}
