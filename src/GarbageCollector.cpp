#include "GarbageCollector.hpp"
#include "Environment.hpp"

#include <print>

jl::GarbageCollector::GarbageCollector(EnvRef global, EnvRef curr, std::vector<Environment*> env_stack)
    : m_global { global }
    , m_curr { curr }
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
            delete to_be_deleted;

#ifdef NDEBUG
            delete_count += 1;
#endif
        }
    }
}

jl::GarbageCollector::~GarbageCollector()
{
    collect();
    collect();

#ifdef NDEBUG
    std::println("Total allocations  : {}", alloc_count);
    std::println("Total deallocations: {}", delete_count);
#endif
}