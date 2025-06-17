// #pragma once

// #include "Chunk.hpp"
// #include "Value.hpp"
// #include <cstdint>
// #include <stack>
// namespace jl {

// class VM {
// public:
//     enum InterpretResult {
//         OK,
//         COMPILER_ERROR,
//         RUNTIME_ERROR,
//     };

//     InterpretResult run(const Chunk& chunk);

// private:
//     uint32_t m_ip { 0 };
//     std::stack<Value> m_stack;
// };

// }