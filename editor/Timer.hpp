#pragma once

namespace jed {

class Timer {
public:
    Timer(float limit);
    ~Timer() = default;

    bool update(float delta);
    bool finished();
    void reset();
private:
    float m_curr_time = 0.0f;
    float m_limit = 0.0f;
};

} // namespace jed
