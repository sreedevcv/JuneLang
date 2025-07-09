#pragma once

#include "Environment.hpp"
#include "MemoryPool.hpp"

namespace jl {

class GarbageCollector : public MemoryPool {
public:
    using EnvRef = Environment*&;

    GarbageCollector(EnvRef global, EnvRef curr, std::vector<Environment*>& env_stack);
    ~GarbageCollector();

    // template <CanBeRef T, typename... Args>
    template <typename T, typename... Args>
    T* allocate(Args&&... args)
    {
        alloc_count += 1;
        // Mark all
        mark(m_global);
        mark(m_curr);

        for (auto e : m_env_stack) {
            mark(e);
        }

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
        /* We do this to maintain a reference to the temporary vaiable that are allocated
        * One place where this happens is during a call statement where the arguments are
        * evaluated and before their refrences are stored inside a environment, a new 
        * environment is allocated in Callable::call which causes the previous allocations
        * to be deleted 
        */
        m_curr->m_refs.insert(obj);
        return obj;
    }

private:
    EnvRef m_global;
    EnvRef m_curr;
    std::vector<Environment*>& m_env_stack;

    int alloc_count { 0 };
#ifdef MEM_DEBUG
    DebugArena m_arena;
    int delete_count { 0 };
#endif

    void collect();
};

}