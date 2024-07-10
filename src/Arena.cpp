#include "Arena.hpp"

#include <cstdlib>

jl::Arena::Arena(uint64_t size)
    : m_size(size)
{
    m_memory = malloc(m_size);
}

jl::Arena::~Arena()
{
    for (auto& dtor : m_dtors) {
        dtor();
    }
    free(m_memory);
    std::cout << "[" << (long)(m_memory) % 1000 << "] " << m_ptr << " bytes freed" << std::endl;
}

bool jl::Arena::is_full()
{
    return m_ptr >= m_size;
}
