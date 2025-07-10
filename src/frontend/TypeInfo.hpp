#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace jl {

struct TypeInfo {
    std::string name;
    bool is_array {false};
    std::optional<int32_t> size;    // Set if array size is give
};

}