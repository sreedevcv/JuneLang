#include "Arena.hpp"

#include <cstdlib>

jl::Arena::Arena(uint32_t size)
    : m_size(size)
{
    m_memory = malloc(m_size);
}

jl::Arena::~Arena()
{
    for (auto& dtor: m_dtors) {
        dtor();
    }
    free(m_memory);
}

bool jl::Arena::is_full()
{
    return m_ptr >= m_size;
}
