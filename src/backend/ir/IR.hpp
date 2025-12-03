#pragma once

#include <cstdint>
#include <string>

namespace jl {
namespace ir {
    class IR {
    public:
        IR(uint32_t line);
        virtual ~IR() = default;

        virtual std::string to_str() const = 0;

        uint32_t line() const;

    private:
        uint32_t m_line;
    };
}
}
