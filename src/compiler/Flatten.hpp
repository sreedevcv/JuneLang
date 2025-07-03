#pragma once

#include <map>
#include <unordered_map>
#include <vector>

#include "Chunk.hpp"
#include "Ir.hpp"

namespace jl {
std::pair<
    std::vector<jl::Ir>,
    std::vector<uint32_t>>
flatten(const std::map<std::string, Chunk>& chunks);

std::ostream& disassemble(
    std::ostream& in,
    const std::vector<Ir>& irs,
    const std::vector<uint32_t>& lines);
}