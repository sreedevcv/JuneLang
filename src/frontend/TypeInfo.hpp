#pragma once

#include <cstdint>
#include <string>

namespace jl {

struct TypeInfo {
    std::string name;
    bool is_array {false};
    int32_t size;
};

}