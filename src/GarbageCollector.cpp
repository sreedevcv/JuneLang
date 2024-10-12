#include "GarbageCollector.hpp"
#include <iostream>

jl::GarbageCollector::GarbageCollector(EnvRef global, EnvRef curr)
    : m_global { global }
    , m_curr { curr }
{
}

void jl::GarbageCollector::collect()
{
    auto ptr = m_head->m_next;

    while (ptr) {
        if (ptr->m_marked) {
            ptr->m_marked = false;
            ptr = ptr->m_next;
        } else {
            auto to_be_deleted = ptr;
            ptr = ptr->m_next;
            
            if (to_be_deleted == nullptr) {
                std::cout << "Already null!!!" << std::endl;
            }
            delete to_be_deleted;
        }
    }
}

jl::GarbageCollector::~GarbageCollector()
{
    mark(m_global);
    mark(m_curr);
    collect();
}