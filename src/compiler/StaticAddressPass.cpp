#include "StaticAddressPass.hpp"

void jl::patch_memmory_address(std::map<std::string, Chunk>& m_chunk_map, uint64_t base_address)
{
    // Add the base address to all operands in ir which is a pointer type
    for (auto& [name, chunk] : m_chunk_map) {
        for (auto& ir : chunk.get_ir_mut()) {
            const auto type = ir.type();
            if (type == Ir::UNARY && is_pure_ptr(get_type(ir.unary().operand))) {
                std::get<PtrVar>(std::get<UnaryIr>(ir.data).operand).offset += base_address;
            } else if (type == Ir::JUMP_STORE && is_pure_ptr(get_type(ir.jump().target))) {
                std::get<PtrVar>(std::get<JumpIr>(ir.data).target).offset += base_address;
            } else if (type == Ir::CONTROL && is_pure_ptr(get_type(ir.control().data))) {
                std::get<PtrVar>(std::get<ControlIr>(ir.data).data).offset += base_address;
            }
        }
    }
}
