#include "Optimizer.hpp"

#include "Function.hpp"
#include "LiteralValue.hpp"
#include "Utils.hpp"
#include "ir/AllocateList.hpp"
#include "ir/Binary.hpp"
#include "ir/Call.hpp"
#include "ir/ConditionalJump.hpp"
#include "ir/DebugPrint.hpp"
#include "ir/IR.hpp"
#include "ir/InitLiteral.hpp"
#include "ir/Jump.hpp"
#include "ir/Move.hpp"
#include "ir/Phi.hpp"
#include "ir/Read.hpp"
#include "ir/Return.hpp"
#include "ir/TypeCast.hpp"
#include "ir/Unary.hpp"
#include "ir/Write.hpp"
#include "utils/algorithms.hpp"
#include <print>
#include <queue>
#include <stack>

enum LatticeType {
    TOP,
    CONSTANT,
    BOTTOM,
};

struct LatticeValue {
    LatticeType type;
    std::optional<jl::LiteralValue> value;

    [[nodiscard]]
    LatticeValue meet(const LatticeValue& other) const
    {
        // a ^ BOTTOM = BOTTOM; a >= BOTTOM
        if (type == BOTTOM || other.type == BOTTOM) {
            return { BOTTOM, std::nullopt };
        } // a ^ TOP = TOP; TOP >= a
        else if (type == TOP) {
            return other;
        } else if (other.type == TOP) {
            return *this;
        } else {
            // // ci ^ cj = ci if ci == cj
            // if (value && other.value && *value == *other.value) {
            //     return { CONSTANT, value };
            // } // ci ^ cj = BOT if ci != cj
            // else {
            //     return { BOTTOM, std::nullopt };
            // }
            return { CONSTANT, std::nullopt };
        }
    }

    std::string to_str() const
    {
        std::string out = "";
        switch (type) {
        case TOP:
            return "TOP";
        case CONSTANT:
            return "CNT[" + value->to_str() + "]";
        case BOTTOM:
            return "BOT";
        default:
            return "-";
        }
    }
};

using CFGEdge = std::pair<jl::BasicBlock*, jl::BasicBlock*>;

struct CFGEdgeHasher {
    std::size_t operator()(const CFGEdge& edge) const
    {
        auto hash1 = std::hash<const jl::BasicBlock*> {}(edge.first);
        auto hash2 = std::hash<const jl::BasicBlock*> {}(edge.second);
        std::size_t seed = hash1;
        seed ^= hash2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};

using ValueMap = std::unordered_map<jl::value::Variable, LatticeValue, jl::value::VariableHasher>;
using ExecMap = std::unordered_map<CFGEdge, bool, CFGEdgeHasher>;

/// Initializes all the definitions with `TOP` and literal values
/// with `CONSTANT`
ValueMap init_lattice_values(jl::Function* function)
{
    ValueMap lattice_values;

    for (auto& ir : function->irs()) {
        if (auto var = ir->def()) {
            if (auto init = dynamic_cast<jl::ir::InitLiteral*>(ir.get())) {
                lattice_values[*var] = {
                    .type = CONSTANT,
                    .value = init->m_source
                };
            } else {
                lattice_values[*var] = {
                    .type = TOP,
                    .value = std::nullopt
                };
            }
        }
    }

    return lattice_values;
}

class IRVisitorForMeet : jl::ir::IRVisitor {
public:
    LatticeValue value;
    ValueMap& value_map;

    IRVisitorForMeet(jl::ir::IR* ir, LatticeValue init, ValueMap& map)
        : value(init)
        , value_map(map)
    {
        ir->accept(*this);
    }

private:
    void visit_binary_ir(jl::ir::Binary& binary)
    {
        value = value.meet(value_map[binary.m_operand_a]);
        value = value.meet(value_map[binary.m_operand_b]);

        if (value.type == CONSTANT) {
            perform_binary_airthmetic(binary);
        }
    }

    void visit_return_ir(jl::ir::Return& ret)
    {
        if (ret.m_ret_val) {
            value = value.meet(value_map[*ret.m_ret_val]);
        }
    }

    void visit_call_ir(jl::ir::Call& call)
    {
        for (auto arg : call.m_args) {
            value = value.meet(value_map[arg]);
        }
    }

    void visit_unary_ir(jl::ir::Unary& unary)
    {
        value = value.meet(value_map[unary.m_operand]);
        if (value.type == CONSTANT) {
            perform_unary_airthmetic(unary);
        }
    }

    void visit_init_literal_ir(jl::ir::InitLiteral& literal)
    {
        value = value.meet({ CONSTANT, literal.m_source });
    }

    void visit_type_cast_ir(jl::ir::TypeCast& type_cast)
    {
        value = value.meet(value_map[type_cast.m_source]);
    }

    void visit_phi(jl::ir::Phi& phi) { unimplemented("No phis in block iteration"); }

    template <typename Op>
    auto do_op(jl::LiteralValue::type& one, jl::LiteralValue::type& two, bool is_float, Op op)
    {
        using float_t = jl::LiteralValue::float_type;
        using int_t = jl::LiteralValue::int_type;
        using literal = jl::LiteralValue::type;
        return is_float ? literal(op(std::get<float_t>(one), std::get<float_t>(two)))
                        : literal(op(std::get<int_t>(one), std::get<int_t>(two)));
    }

    void perform_binary_airthmetic(jl::ir::Binary& binary)
    {
        auto& op1_data = value_map[binary.m_operand_a].value->data;
        auto& op2_data = value_map[binary.m_operand_b].value->data;

        switch (binary.m_operation) {
        case jl::ir::Binary::PLUS:
            value.value = do_op(op1_data, op2_data, binary.m_is_float, std::plus {});
            break;
        case jl::ir::Binary::MINUS:
            value.value = do_op(op1_data, op2_data, binary.m_is_float, std::minus {});
            break;
        case jl::ir::Binary::STAR:
            value.value = do_op(op1_data, op2_data, binary.m_is_float, std::multiplies {});
            break;
        case jl::ir::Binary::SLASH:
            value.value = do_op(op1_data, op2_data, binary.m_is_float, std::divides {});
            break;
        case jl::ir::Binary::GREATER:
            value.value = do_op(op1_data, op2_data, binary.m_is_float, std::greater {});
            break;
        case jl::ir::Binary::LESS:
            value.value = do_op(op1_data, op2_data, binary.m_is_float, std::less {});
            break;
        case jl::ir::Binary::GREATER_EQUAL:
            value.value = do_op(op1_data, op2_data, binary.m_is_float, std::greater_equal {});
            break;
        case jl::ir::Binary::LESS_EQUAL:
            value.value = do_op(op1_data, op2_data, binary.m_is_float, std::less_equal {});
            break;
        case jl::ir::Binary::EQUAL_EQUAL:
            value.value = do_op(op1_data, op2_data, binary.m_is_float, std::equal_to {});
            break;
        case jl::ir::Binary::BANG_EQUAL:
            value.value = do_op(op1_data, op2_data, binary.m_is_float, std::not_equal_to {});
            break;
        case jl::ir::Binary::PERCENT:
            value.value = jl::LiteralValue::type(std::get<jl::LiteralValue::int_type>(op1_data) % std::get<jl::LiteralValue::int_type>(op2_data));
            break;
        case jl::ir::Binary::BIT_AND:
            value.value = jl::LiteralValue::type(std::get<jl::LiteralValue::int_type>(op1_data) & std::get<jl::LiteralValue::int_type>(op2_data));
            break;
        case jl::ir::Binary::BIT_OR:
            value.value = jl::LiteralValue::type(std::get<jl::LiteralValue::int_type>(op1_data) | std::get<jl::LiteralValue::int_type>(op2_data));
            break;
        case jl::ir::Binary::BIT_XOR:
            value.value = jl::LiteralValue::type(std::get<jl::LiteralValue::int_type>(op1_data) ^ std::get<jl::LiteralValue::int_type>(op2_data));
            break;
        case jl::ir::Binary::LOG_AND:
            value.value = jl::LiteralValue::type(std::get<jl::LiteralValue::bool_type>(op1_data) && std::get<jl::LiteralValue::bool_type>(op2_data));
            break;
        case jl::ir::Binary::LOG_OR:
            value.value = jl::LiteralValue::type(std::get<jl::LiteralValue::bool_type>(op1_data) || std::get<jl::LiteralValue::bool_type>(op2_data));
            break;
        }
    }

    void perform_unary_airthmetic(jl::ir::Unary& unary)
    {
        auto& data = value_map[unary.m_operand].value->data;
        bool is_float = static_cast<const jl::type::Builtin*>(unary.m_operand.type())->m_primitive == jl::type::Builtin::FLOAT ? true : false;

        switch (unary.m_operation) {
        case jl::ir::Unary::MINUS:
            value.value = is_float ? jl::LiteralValue::type(-std::get<jl::LiteralValue::float_type>(data))
                                   : jl::LiteralValue::type(-std::get<jl::LiteralValue::int_type>(data));
            break;
        case jl::ir::Unary::BANG:
            value.value = jl::LiteralValue::type(!std::get<jl::LiteralValue::bool_type>(data));
            break;
        case jl::ir::Unary::BIT_NOT:
            value.value = jl::LiteralValue::type(~std::get<jl::LiteralValue::int_type>(data));
            break;
        }
    }

    void visit_allocate_list_ir(jl::ir::AllocateList& allocate) { }
    void visit_allocate_var_ir(jl::ir::AllocateVar& allocate) { }
    void visit_read_ir(jl::ir::Read& read) { }
    void visit_write_ir(jl::ir::Write& write) { }
    void visit_label_ir(jl::ir::Label& label) { }
    void visit_move_ir(jl::ir::Move& move) { }
    void visit_jump_ir(jl::ir::Jump& jump) { }
    void visit_cond_jump_ir(jl::ir::CondJump& jump) { }
    void visit_debug_print_ir(jl::ir::DebugPrint& print) { }
};

struct SCCPState {
    ExecMap exec_map;
    jl::Function* function;
    ValueMap lattice_values;
    std::queue<CFGEdge> flow_work_list;
    std::queue<jl::ir::IR*> ssa_work_list;

    SCCPState(jl::Function* function)
        : function(function)
        , lattice_values(init_lattice_values(function))
        , exec_map(init_cfg_edges(function))
    {
        flow_work_list.push({ function->entry_block(), function->entry_block() });
    }

    // Meet over the operands of phi
    void visit_phi(jl::ir::Phi* phi)
    {
        if (lattice_values[phi->m_dest].type == BOTTOM) {
            // Nothing much to do since we have already seen that this variable could hold
            // any/multiple values during runtime.
            //
            return;
        }

        LatticeValue acc = { TOP, std::nullopt };

        std::println("\t-visit_phi: {}, current val: ({})", phi->to_str(), lattice_values[phi->m_dest].to_str());

        for (auto [val, blk] : phi->m_opers) {
            // Only select the value if the edge has been already executed.
            // Otherwise meet with TOP for unexecuted edges
            //
            if (exec_map[{ blk, phi->parent }]) {
                std::println("\t\t* from exec blk: {}", lattice_values[val].to_str());
                acc = acc.meet(lattice_values[val]);
            } else {
                acc = acc.meet({ TOP, std::nullopt });
            }
        }

        std::println("\t- Final meet: {}, Current: {} ", acc.to_str(), lattice_values[phi->m_dest].to_str());

        // NOTE replace wiht overloaded ==
        if (acc.type != lattice_values[phi->m_dest].type) {
            lattice_values[phi->m_dest] = acc;
            add_uses_to_ssa_worklist(phi->m_dest, phi->parent);
        }
    }

    // Add all uses of a ssa value to the ssa worklist only if the edge has been executed
    void add_uses_to_ssa_worklist(jl::value::Variable def, jl::BasicBlock* curr_block)
    {
        std::println("\t-Adding uses for {}", def.to_str());

        for (auto& ir : function->irs()) {
            if (ir->uses(def) && exec_map[{ curr_block, ir->parent }]) {
                std::println("\t\t*Added {}", ir->to_str());
                ssa_work_list.push(ir.get());
            }
        }
    }

    // Visits an expression.
    //
    // If its a conditional jump where the condition has already been evaluated,
    // then add the computed branch to work list. Otherwise add both the branches
    // to the worklist.
    //
    // If its a normal jump, then add the target to worklist.
    //
    // For normal expressions that have a def, compute the meet over its operand and then
    // if the value changes, add all the uses to the ssa work list
    void visit_expression(jl::ir::IR* ir)
    {
        std::println("\t-visit_expr: {}", ir->to_str());

        if (auto jump = dynamic_cast<jl::ir::CondJump*>(ir)) {
            auto lat_val = lattice_values[jump->m_condition];

            switch (lat_val.type) {
            case CONSTANT: {
                bool const_val = std::get<jl::LiteralValue::bool_type>(lat_val.value->data);
                std::println("\t\t*CondJump - CONSTANT: selecting {} branch", const_val);

                if (const_val == true) {
                    flow_work_list.push({ ir->parent, jump->m_true_target });
                } else {
                    flow_work_list.push({ ir->parent, jump->m_false_target });
                }
            } break;
            case BOTTOM:
                std::println("\t\t*CondJump - BOTTOM: selecting both branches");
                flow_work_list.push({ ir->parent, jump->m_true_target });
                flow_work_list.push({ ir->parent, jump->m_false_target });
                break;
            case TOP:
                unimplemented("no idea what to do");
                break;
            }
        } else if (auto jump = dynamic_cast<jl::ir::Jump*>(ir)) {
            std::println("\t\t*Jump");
            flow_work_list.push({ ir->parent, jump->m_target });
        } else if (auto def = ir->def()) {

            if (lattice_values[*def].type == BOTTOM) {
                // Nothing much to do since we have already seen that this variable could hold
                // any/multiple values during runtime.
                return;
            }

            auto new_lattice = IRVisitorForMeet(ir, { TOP, std::nullopt }, lattice_values).value;
            auto original_value = lattice_values[*ir->def()];

            std::println("\t\t-Considering {}", ir->to_str());
            std::println("\t\t-Original: {}, new: {}", original_value.to_str(), new_lattice.to_str());

            // if the computed value is different from the current value
            // NOTE replace wiht overloaded ==
            if (new_lattice.type != original_value.type) {
                lattice_values[*def] = new_lattice;
                add_uses_to_ssa_worklist(*def, ir->parent);
            }
        }
    }

    /// Traverses the CFG and assigns `false` to each edge
    ExecMap init_cfg_edges(jl::Function* function)
    {
        ExecMap exec_map;
        std::stack<jl::BasicBlock*> stk;
        std::unordered_set<jl::BasicBlock*> visited;
        stk.push(function->entry_block());

        while (!stk.empty()) {
            auto node = stk.top();
            stk.pop();

            if (visited.contains(node)) {
                continue;
            }

            visited.insert(node);

            const auto [left, right] = jl::algorithms::get_sucessors(node);

            if (left != nullptr) {
                exec_map[{ node, left }] = false;
                stk.push(left);
            }
            if (right != nullptr) {
                exec_map[{ node, right }] = false;
                stk.push(right);
            }
        }

        return exec_map;
    }
};

void jl::opt::sccp(jl::Function* function)
{
    SCCPState state(function);
    auto predecessors = jl::algorithms::get_predecessors(function);

    while (!state.flow_work_list.empty() || !state.ssa_work_list.empty()) {
        while (!state.flow_work_list.empty()) {
            auto edge = state.flow_work_list.front();
            state.flow_work_list.pop();

            if (state.exec_map[edge]) {
                continue;
            }

            state.exec_map[edge] = true;
            auto [src, dest] = edge;

            for (auto phi : dest->phis) {
                state.visit_phi(phi);
            }

            for (auto ir = dest->head; ir != nullptr; ir = ir->next) {
                state.visit_expression(ir);
            }
        }
        while (!state.ssa_work_list.empty()) {
            auto ssa = state.ssa_work_list.front();
            state.ssa_work_list.pop();

            if (auto phi = dynamic_cast<ir::Phi*>(ssa)) {
                state.visit_phi(phi);
            } else {
                state.visit_expression(ssa);
            }
        }
    }

    std::println("Final Lattice Values:");
    for (const auto [var, val] : state.lattice_values) {
        std::println("{} -> {}", var.to_str(), val.to_str());
    }

    std::println("\nFinal ExecMap Values:");
    for (const auto [edge, flag] : state.exec_map) {
        std::println("{} -> {}: {}", edge.first->get_name(), edge.second->get_name(), flag);
    }
}