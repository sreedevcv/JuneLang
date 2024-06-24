#include "Timer.hpp"



jed::Timer::Timer(float limit)
    : m_limit(limit)
{
}

bool jed::Timer::update(float delta)
{
    m_curr_time += delta;
    return finished();
}

bool jed::Timer::finished()
{
    return m_curr_time >= m_limit;
}

void jed::Timer::reset()
{
    m_curr_time = 0;
}
