#pragma once

#include <cstdint>
#include <functional>
#include <iostream>
#include <vector>

namespace jl {

class Arena {
public:
    Arena(uint64_t size);
    ~Arena();

    template <typename T, typename... Args>
    T* allocate(Args&&... args)
    {
        if (sizeof(T) + m_ptr >= m_size) {
            std::cout << "Fatal Error: No free spcac to allocate in arena" << std::endl;
            std::exit(1);
        }

        void* new_addr = static_cast<void*>(static_cast<char*>(m_memory) + m_ptr);
        T* ptr = static_cast<T*>(new_addr);
        // ptr = std::construct_at(ptr, args...);
        ptr = new (ptr) T(std::forward<Args>(args)...);
        m_ptr += sizeof(T);

        m_dtors.push_back([=]() {
            ptr->~T();
        });

        // std::cout << "[" << (long)(m_memory) % 1000  << "] Allocated " << sizeof(T) << " bytes\n";

        return ptr;
    }

    bool is_full();

protected:
    uint64_t m_size;
    uint64_t m_ptr = 0;
    void* m_memory;

    using destructor_t = std::function<void(void)>;

    std::vector<destructor_t> m_dtors;
};

} // namespace jl
