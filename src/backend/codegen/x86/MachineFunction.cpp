#include "MachineFunction.hpp"
#include "Instruction.hpp"

#include "Function.hpp"
#include "codegen/x86/MachineBlock.hpp"
#include "codegen/x86/Register.hpp"
#include "ir/AllocateVar.hpp"
#include "types/Type.hpp"
#include <algorithm>
#include <cstdint>
#include <memory>
#include <sstream>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>

jl::x86::MachineFunction::MachineFunction(const std::string& name, Function* function)
    : m_name(name)
{
    // Calculate stack offsets
    int32_t offset = 0;

    for (auto& block : function->blocks()) {
        for (auto ir = block->head; ir != nullptr; ir = ir->next) {
            if (auto alloca = dynamic_cast<ir::AllocateVar*>(ir)) {
                const auto alignment = alloca->m_addr.type()->alignment();

                if (offset % alignment != 0) {
                    offset = ((offset + alignment - 1) / alignment) * alignment;
                }

                m_stk_offset[alloca->m_addr.id()] = offset;
                offset += alloca->m_addr.type()->size();
            }
        }
    }

    total_stack_space = offset;

    // Assign virtual registers for pyhsical registers
    m_physical_register_map[PhysicalRegister::rax] = new_register();
    m_physical_register_map[PhysicalRegister::rbp] = new_register();
    m_physical_register_map[PhysicalRegister::rsp] = new_register();

    set_allocation(m_physical_register_map[PhysicalRegister::rax], PhysicalRegister(PhysicalRegister::rax));
    set_allocation(m_physical_register_map[PhysicalRegister::rbp], PhysicalRegister(PhysicalRegister::rbp));
    set_allocation(m_physical_register_map[PhysicalRegister::rsp], PhysicalRegister(PhysicalRegister::rsp));
}

jl::x86::MachineFunction::~MachineFunction() = default;

jl::x86::VirtualRegister jl::x86::MachineFunction::new_register(bool is_float)
{
    return VirtualRegister(m_reg_count++, is_float);
}

jl::x86::VirtualRegister& jl::x86::MachineFunction::get_register(value::Variable var)
{
    if (!m_register_map.contains(var)) {
        m_register_map[var] = new_register(type::is_float(var.type()));
    }

    return m_register_map[var];
}

jl::x86::MachineBlock* jl::x86::MachineFunction::get_block(const std::string& name)
{
    if (m_block_map.contains(name)) {
        return m_block_map[name];
    } else {
        m_blocks.push_back(std::make_unique<MachineBlock>(MachineBlock(name)));
        m_block_map[name] = m_blocks.back().get();
        return m_blocks.back().get();
    }
}

std::list<std::unique_ptr<jl::x86::MachineBlock>>& jl::x86::MachineFunction::blocks()
{
    return m_blocks;
}

std::vector<jl::x86::VirtualRegister>& jl::x86::MachineFunction::inputs()
{
    return m_inputs;
}

std::string jl::x86::MachineFunction::to_str() const
{
    std::stringstream ss;
    ss << "; Function\n";
    ss << m_name << ": \n";

    for (const auto& block : m_blocks) {
        ss << block->to_str();
        ss << "\n";
    }

    ss << "\n";
    return ss.str();
}

int32_t jl::x86::MachineFunction::get_ssa_offset(value::Variable var) const
{
    return m_stk_offset.at(var.id());
}

std::unordered_map<jl::x86::MachineBlock*, std::vector<jl::x86::MachineBlock*>> jl::x86::MachineFunction::predecessors() const
{
    std::unordered_map<MachineBlock*, std::vector<MachineBlock*>> preds;

    for (auto& block : m_blocks) {
        auto successors = block->successors();

        for (auto succ : successors) {
            preds[succ].push_back(block.get());
        }
    }

    return preds;
}

std::unordered_map<jl::x86::MachineBlock*, std::vector<jl::x86::MachineBlock*>> jl::x86::MachineFunction::successors() const
{

    std::unordered_map<MachineBlock*, std::vector<MachineBlock*>> succ;

    for (auto& block : m_blocks) {
        succ[block.get()] = block->successors();
    }

    return succ;
}

std::vector<jl::x86::MachineBlock*> jl::x86::MachineFunction::rpo() const
{
    std::unordered_set<MachineBlock*> visited;
    std::stack<MachineBlock*> stk;
    std::vector<MachineBlock*> post_order;
    auto entry_block = m_blocks.front().get();

    if (entry_block) {
        stk.push(entry_block);
        visited.insert(entry_block);
    }

    while (!stk.empty()) {
        auto node = stk.top();

        auto successors = node->successors();

        // Find an unvisited child to descend into
        bool pushed = false;

        for (auto succ : successors) {
            if (succ != nullptr && !visited.contains(succ)) {
                visited.insert(succ);
                stk.push(succ);
                pushed = true;
            }
        }

        if (!pushed) {
            // All children processed — emit this node
            post_order.push_back(node);
            stk.pop();
        }
    }

    std::reverse(post_order.begin(), post_order.end());

    return post_order;
}

jl::x86::VirtualRegister jl::x86::MachineFunction::get_physical_register(PhysicalRegister::Type reg) const
{
    return m_physical_register_map.at(reg);
}

std::string jl::x86::MachineFunction::text() const
{
    std::stringstream ss;
    ss << "; Function\n";
    ss << m_name << ": \n";

    VirtualRegister::debug_print = false;

    for (const auto& block : m_blocks) {
        ss << block->text();
        ss << "\n";
    }

    VirtualRegister::debug_print = true;

    ss << "\n";
    return ss.str();
}

void jl::x86::MachineFunction::set_allocation(jl::x86::VirtualRegister reg, jl::x86::MachineAlloc alloc)
{
    m_allocations[reg] = alloc;
}

std::optional<jl::x86::MachineAlloc> jl::x86::MachineFunction::get_allocation(jl::x86::VirtualRegister reg) const
{
    if (!m_allocations.contains(reg)) {
        return std::nullopt;
    }
    return m_allocations.at(reg);
}

std::optional<jl::value::Variable> jl::x86::MachineFunction::get_variable(VirtualRegister reg) const
{
    for (auto& [var, vreg] : m_register_map) {
        if (vreg.id == reg.id) {
            return var;
        }
    }

    return std::nullopt;
}

const std::string& jl::x86::MachineFunction::name() const
{
    return m_name;
}
