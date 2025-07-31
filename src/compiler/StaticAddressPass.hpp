#pragma once

#include "Chunk.hpp"
#include <map>
#include <string>
namespace jl {

void patch_memmory_address(std::map<std::string, Chunk>& m_chunk_map, uint64_t base_address);

}