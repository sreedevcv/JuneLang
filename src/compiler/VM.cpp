// #include "VM.hpp"
// #include "OpCode.hpp"
// #include "Value.hpp"
// #include <cstdint>
// #include <cstdlib>
// #include <iostream>
// #include <print>
// #include <stack>

// static jl::Value pop(std::stack<jl::Value>& stk)
// {
//     jl::Value constant = std::move(stk.top());
//     stk.pop();

//     return constant;
// }

// jl::VM::InterpretResult jl::VM::run(const Chunk& chunk)
// {
//     m_ip = 0;

//     while (true) {
//         auto instruction = chunk.read_byte<OpCode>(m_ip++);

//         chunk.disasemble_opcode(std::cout, m_ip);

//         switch (instruction) {
//         case OpCode::RETURN: {
//             auto constant = pop(m_stack);
//             std::println("{}", stringify(&constant));
//             return OK;
//         }
//         case OpCode::CONSTANT: {
//             const auto offset = chunk.read_byte<uint8_t>(m_ip++);
//             auto constant = chunk.read_constant(offset);
//             m_stack.push(std::move(constant));
//             break;
//         }
//         case OpCode::NEGATE: {
//             auto constant = pop(m_stack);
            
//             switch (get_type(constant)) {
//             case Type::INT: {
//                 int n = std::get<int>(constant.get());
//                 constant.get() = n * -1;
//                 break;
//             }
//             case Type::FLOAT: {
//                 double n = std::get<double>(constant.get());
//                 constant.get() = n * -1;
//                 break;
//             }

//             default:
//                 std::println("Cannot negate a non number: {}", stringify(&constant));
//                 std::exit(1);
//             }

//             m_stack.push(constant);
//             break;
//         }
//         }
//     }
// }
