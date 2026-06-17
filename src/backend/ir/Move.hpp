// #pragma once

// #include "backend/ir/IR.hpp"
// #include "backend/value/Variable.hpp"

// #include <cstdint>

// namespace jl {
// namespace ir {
//     struct Move : public IR {
//         Move(value::Variable source,
//             value::Variable dest,
//             uint32_t line);

//         virtual ~Move() = default;

//         std::string to_str() const override;

//         void accept(IRVisitor& visitor) override;

//         bool uses(value::Variable var) override;

//         value::Variable m_source;
//         value::Variable m_dest;
//     };
// }
// }
