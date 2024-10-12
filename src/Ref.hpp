#pragma once

namespace jl {

class Ref {
public:
    Ref* next { nullptr };
    bool marked { false };
};

}