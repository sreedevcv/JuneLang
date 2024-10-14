#pragma once

#include "Environment.hpp"
#include "MemoryPool.hpp"
#include <stack>

#define NDEBUG

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

        T* obj = new T(std::forward<Args>(args)...);
        obj->m_next = m_head->m_next;
        m_head->m_next = obj;
        return obj;
    }

private:
    EnvRef m_global;
    EnvRef m_curr;
    std::vector<Environment*> m_env_stack;

    void collect();

#ifdef NDEBUG
    int alloc_count { 0 };
    int delete_count { 0 };
#endif
};

}