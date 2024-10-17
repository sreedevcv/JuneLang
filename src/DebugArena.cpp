#include "DebugArena.hpp"

#include <print>
#include "Ref.hpp"

jl::DebugArena::DebugArena(uint64_t size)
	: Arena(size)
{
}

void jl::DebugArena::print_memory_layout()
{

	for (int i = 0; i < m_alloc_sizes.size(); i++) {
		std::print("[{:4>}: {:3>}] ", i,  m_alloc_sizes[i]);
	}
	std::println();

	int size_till_now = 0;
	for (int i = 0; i < m_alloc_sizes.size(); i++) {
		Ref* temp = (Ref*)((uint8_t*)m_memory + size_till_now);
		std::print("[{:8>}] ", dynamic_cast<Ref*>(temp)->m_marked);
		size_till_now += m_alloc_sizes[i];
	}
	std::println();
#ifdef MEM_DEBUG
	size_till_now = 0;
	for (int i = 0; i < m_alloc_sizes.size(); i++) {
		Ref* temp = (Ref*)((uint8_t*)m_memory + size_till_now);
		std::print("[{:8>}] ", dynamic_cast<Ref*>(temp)->in_use);
		size_till_now += m_alloc_sizes[i];
	}
	std::println();
#endif
}
