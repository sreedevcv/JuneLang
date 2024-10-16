#pragma once

#include "Environment.hpp"
#include "MemoryPool.hpp"
#include "DebugArena.hpp"

#include <print>

namespace jl {

class GarbageCollector : public MemoryPool {
public:
    using EnvRef = Environment*&;

    GarbageCollector(EnvRef global, EnvRef curr);
    ~GarbageCollector();

    template <CanBeRef T, typename... Args>
    T* allocate(Args&&... args)
    {
        alloc_count += 1;
        // Mark all
        mark(m_global);
        mark(m_curr);

#ifdef MEM_DEBUG
        m_arena.print_memory_layout();
#endif 

        collect();


#ifdef MEM_DEBUG
        T* obj = m_arena.allocate<T, Args...>(std::forward<Args>(args)...);
#else
        T* obj = new T(std::forward<Args>(args)...);
        //std::println("Allocated {} bytes", sizeof(T));
#endif
        obj->m_next = m_head->m_next;
        m_head->m_next = obj;
        obj->m_gc = true;
        m_curr->m_refs.insert(obj);
        return obj;
    }

private:
    EnvRef m_global;
    EnvRef m_curr;

    int alloc_count { 0 };
#ifdef MEM_DEBUG
    DebugArena m_arena;
    int delete_count { 0 };
#endif

    void collect();

};

}