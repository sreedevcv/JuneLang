#pragma once

#include <unordered_map>
#include <vector>

#include "Chunk.hpp"
#include "Ir.hpp"

namespace jl {
std::vector<Ir> flatten(const std::unordered_map<std::string, Chunk>& chunks);
}