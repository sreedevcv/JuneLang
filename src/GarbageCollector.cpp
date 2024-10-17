#include "GarbageCollector.hpp"
#include "Environment.hpp"

#include "Callable.hpp"

#include <print>

jl::GarbageCollector::GarbageCollector(EnvRef global, EnvRef curr, std::vector<Environment*>& env_stack)
    : m_global { global }
    , m_curr { curr }
    , m_env_stack(env_stack)
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
            //std::printf("deleted %p\n", to_be_deleted);
            //if (!dynamic_cast<JlValue*>(to_be_deleted)) {
            //    std::println("not value");
            //}
            //if (dynamic_cast<JlValue*>(to_be_deleted) && static_cast<JlValue*>(to_be_deleted)->m_type == JlValue::INT) {
            //    std::println("Type: {}", (int)(static_cast<JlInt*>(to_be_deleted)->m_type));
            //}

            delete to_be_deleted;
#endif
        }
    }

    m_global->m_marked = false;
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