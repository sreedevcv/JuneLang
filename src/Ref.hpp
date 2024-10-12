#pragma once

namespace jl {

class Ref {
public:
    Ref* m_next { nullptr };
    bool m_marked { false };

    virtual ~Ref() = default;
};

}