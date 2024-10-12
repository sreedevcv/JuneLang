#pragma once

#include "Environment.hpp"
#include "MemoryPool.hpp"

namespace jl {

class GarbageCollector : public MemoryPool {
public:
    using EnvRef = Environment*&;

    GarbageCollector(EnvRef global, EnvRef curr);
    ~GarbageCollector();

    template <CanBeRef T, typename... Args>
    T* allocate(Args&&... args)
    {
        // Mark all
        mark(m_global);
        mark(m_curr);

        collect();

        T* obj = new T(std::forward<Args>(args)...);
        obj->m_next = m_head->m_next;
        m_head->m_next = obj;
        return obj;
    }

private:
    EnvRef m_global;
    EnvRef m_curr;

    void collect();
};

}