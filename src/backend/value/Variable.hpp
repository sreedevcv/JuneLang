#pragma once

#include "Value.hpp"
#include <cstdint>
#include <string>

namespace jl {
namespace value {
    class Variable : Value {
    public:
        enum Storage {
            TEMP,
            STACK,
        };

        Variable(uint32_t id, Storage storage);

        std::string to_str() const;

    private:
        uint32_t m_id;
        Storage m_storage;
    };
}
}
