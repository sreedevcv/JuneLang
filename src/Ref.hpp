#pragma once

//#define MEM_DEBUG

namespace jl {

class Ref {
public:
    Ref* m_next { nullptr };
    bool m_marked { false };
    bool m_gc{ false };

    virtual ~Ref() = default;

#ifdef MEM_DEBUG
    bool in_use{ true };
#endif // MEM_DEBUG

};

}