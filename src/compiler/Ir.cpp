#include "Ir.hpp"

#include "Operand.hpp"
#include "Utils.hpp"

jl::Ir::Type jl::get_type(const jl::Ir& ir)
{
    switch (ir.data.index()) {
    case 0:
        return jl::Ir::UNARY;
    case 1:
        return jl::Ir::BINARY;
    case 2:
        return jl::Ir::CONTROL;
    case 3:
        return jl::Ir::JUMP;
    case 4:
        return jl::Ir::CALL;
    default:
        unimplemented();
    }

    return static_cast<jl::Ir::Type>(1000);
}

jl::Ir::Type jl::Ir::type() const
{
    return jl::get_type(*this);
}

const jl::BinaryIr& jl::Ir::binary() const
{
    return std::get<BinaryIr>(data);
}

const jl::UnaryIr& jl::Ir::unary() const
{
    return std::get<UnaryIr>(data);
}

const jl::ControlIr& jl::Ir::control() const
{
    return std::get<ControlIr>(data);
}

const jl::JumpIr& jl::Ir::jump() const
{
    return std::get<JumpIr>(data);
}

const jl::CallIr& jl::Ir::call() const
{
    return std::get<CallIr>(data);
}

const jl::OpCode& jl::Ir::opcode() const
{
    switch (type()) {
    case BINARY:
        return binary().opcode;
    case UNARY:
        return unary().opcode;
    case CONTROL:
        return control().opcode;
    case JUMP:
        return jump().opcode;
    case CALL:
        return call().opcode;
    default:
        unimplemented();
    }
}

const jl::TempVar& jl::Ir::dest() const
{
    switch (type()) {
    case BINARY:
        return binary().dest;
    case UNARY:
        return unary().dest;
    case CALL:
        return call().dest;
    default:
        unimplemented();
    }
}