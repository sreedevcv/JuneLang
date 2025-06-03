#pragma once

#include "Arena.hpp"

namespace jl {

class DebugArena : public Arena {
public:

	DebugArena(uint64_t size);

	template <typename T, typename... Args>
	T* allocate(Args&&... args) {
		T* ptr = Arena::allocate<T>(std::forward<Args>(args)...);
		m_alloc_sizes.push_back(sizeof(T));
		return ptr;
	}

	void print_memory_layout();

private:
	std::vector<int> m_alloc_sizes;

};

}