#pragma once

#include "Arena.hpp"

namespace jl {

class DebugArena : public Arena {
public:

	DebugArena(uint64_t size);

	template <typename T, typename... Args>
	T* allocate(Args&&... args)
	{
		T* ptr = Arena::allocate<T>(std::forward<Args>(args)...);
		return ptr;
	}
};

}