#include "GarbageCollector.hpp"
#include "Environment.hpp"

#include <print>

jl::GarbageCollector::GarbageCollector(EnvRef global, EnvRef curr, std::vector<Environment*> env_stack)
    : m_global { global }
    , m_curr { curr }
#ifdef MEM_DEBUG
    , m_arena {1000}
#endif
{
}

void jl::GarbageCollector::collect()
{
    auto ptr = m_head->m_next;
    auto prev = m_head;

    while (ptr) {
        if (ptr->m_marked) {
            ptr->m_marked = false;
            prev = ptr;
            ptr = ptr->m_next;
        } else {
            auto to_be_deleted = ptr;
            prev->m_next = ptr->m_next;
            ptr = ptr->m_next;

#ifdef MEM_DEBUG
            delete_count += 1;
            to_be_deleted->in_use = false;
#else
            std::printf("deleted %p\n", to_be_deleted);
            delete to_be_deleted;
#endif
        }
    }

    // Make all non-gc allocated vars in envs as not-marked

    for (auto e : m_env_stack) {
        //for (auto& [key, val]: e->)
    }
}

jl::GarbageCollector::~GarbageCollector()
{
    collect();
    collect();

#ifdef MEM_DEBUG
    m_arena.print_memory_layout();
    std::println("Total allocations  : {}", alloc_count);
    std::println("Total deallocations: {}", delete_count);
#endif
}