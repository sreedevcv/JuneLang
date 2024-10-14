#pragma once

#include "Environment.hpp"
#include "MemoryPool.hpp"
#include "DebugArena.hpp"

#define NDEBUG
#define MEM_DEBUG

namespace jl {

class GarbageCollector : public MemoryPool {
public:
    using EnvRef = Environment*&;

    GarbageCollector(EnvRef global, EnvRef curr, std::vector<Environment*> env_stack);
    ~GarbageCollector();

    template <CanBeRef T, typename... Args>
    T* allocate(Args&&... args)
    {
        // Mark all
        mark(m_global);
        mark(m_curr);

        for (auto env : m_env_stack) {
            mark(env);
        }

#ifdef NDEBUG
        alloc_count += 1;
#endif

        collect();

#ifdef MEM_DEBUG
        T* obj = m_arena.allocate<T, Args...>(std::forward<Args>(args)...);
#else
        T* obj = new T(std::forward<Args>(args)...);
#endif
        obj->m_next = m_head->m_next;
        m_head->m_next = obj;
        return obj;
    }

private:
    EnvRef m_global;
    EnvRef m_curr;
    std::vector<Environment*> m_env_stack;
#ifdef MEM_DEBUG
    DebugArena m_arena;
#endif

    void collect();

#ifdef NDEBUG
    int alloc_count { 0 };
    int delete_count { 0 };
#endif
};

}