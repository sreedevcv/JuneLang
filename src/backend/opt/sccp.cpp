#include "Optimizer.hpp"

#include "BasicBlock.hpp"
#include "Function.hpp"
#include "LiteralValue.hpp"
#include "Utils.hpp"
#include "ir/AllocateList.hpp"
#include "ir/Binary.hpp"
#include "ir/Call.hpp"
#include "ir/ConditionalJump.hpp"
#include "ir/DebugPrint.hpp"
#include "ir/IR.hpp"
#include "ir/IRVisitor.hpp"
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
#include "value/Variable.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <print>
#include <queue>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

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
            // ci ^ cj = ci if ci == cj
            if (value && other.value && *value == *other.value) {
                return { CONSTANT, value };
            } // ci ^ cj = BOT if ci != cj
            else {
                return { BOTTOM, std::nullopt };
            }
            // return { CONSTANT, std::nullopt };
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

class IRVisitorForMeet : jl::ir::IRVisitor {
public:
    LatticeValue value;
    ValueMap& value_map;

    IRVisitorForMeet(ValueMap& map, LatticeValue value)
        : value_map(map)
        , value(value)
    {
    }

    void fold(jl::ir::IR* ir)
    {
        ir->accept(*this);
    }

private:
    void visit_binary_ir(jl::ir::Binary& binary)
    {
        if (value_map[binary.m_operand_a].type == BOTTOM || value_map[binary.m_operand_b].type == BOTTOM) {
            value = { .type = BOTTOM };
        } else if (value_map[binary.m_operand_a].type == CONSTANT || value_map[binary.m_operand_b].type == CONSTANT) {
            value.type = CONSTANT;
            perform_binary_airthmetic(binary);
        }
    }

    void visit_unary_ir(jl::ir::Unary& unary)
    {
        if (value_map[unary.m_operand].type == CONSTANT) {
            perform_unary_airthmetic(unary);
        }
    }

    void visit_init_literal_ir(jl::ir::InitLiteral& literal)
    {
        value = { CONSTANT, literal.m_source };
    }

    void visit_type_cast_ir(jl::ir::TypeCast& type_cast)
    {
        unimplemented("todo");
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
    void visit_return_ir(jl::ir::Return& ret) { }
    void visit_call_ir(jl::ir::Call& call) { }
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
        // To get the algorithm started. This will also always mark the entry as executed, preventing
        // it from being deleted
        //
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

        // std::println("\t-visit_phi: {}, current val: ({})", phi->to_str(), lattice_values[phi->m_dest].to_str());

        for (auto [val, blk] : phi->m_opers) {
            // std::println("\t\t*val: {}, edge {} -> {}", val.to_str(), blk->get_name(), phi->parent->get_name());

            // Only select the value if the edge has been already executed.
            // Otherwise meet with TOP for unexecuted edges
            //
            if (is_edge_executed(blk, phi->parent)) {
                // std::println("\t\t* from exec blk: {}", lattice_values[val].to_str());
                acc = acc.meet(lattice_values[val]);
            } else {
                acc = acc.meet({ TOP, std::nullopt });
            }
        }

        // std::println("\t- Final meet: {}, Current: {} ", acc.to_str(), lattice_values[phi->m_dest].to_str());

        // NOTE replace wiht overloaded ==
        if (acc.type != lattice_values[phi->m_dest].type) {
            lattice_values[phi->m_dest] = acc;
            add_uses_to_ssa_worklist(phi->m_dest, phi->parent);
        }
    }

    // Add all uses of a ssa value to the ssa worklist only if the edge has been executed
    void add_uses_to_ssa_worklist(jl::value::Variable def, jl::BasicBlock* curr_block)
    {
        // std::println("\t-Adding uses for {}", def.to_str());

        for (auto& ir : function->irs()) {
            // if (ir->uses(def) && is_edge_executed(curr_block, ir->parent)) {
            if (ir->is_used(def)) {
                // std::println("\t\t*Used by: {}, exec: {}", ir->to_str(), is_edge_executed(curr_block, ir->parent));

                if (is_edge_executed(curr_block, ir->parent)) {
                    // std::println("\t\t*Added {}", ir->to_str());
                    ssa_work_list.push(ir.get());
                }
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
        // std::println("\t-visit_expr: {}", ir->to_str());

        if (auto jump = dynamic_cast<jl::ir::CondJump*>(ir)) {
            auto lat_val = lattice_values[jump->m_condition];

            switch (lat_val.type) {
            case CONSTANT: {
                bool const_val = std::get<jl::LiteralValue::bool_type>(lat_val.value->data);
                // std::println("\t\t*CondJump - CONSTANT: selecting {} branch", const_val);

                if (const_val == true) {
                    flow_work_list.push({ ir->parent, jump->m_true_target });
                } else {
                    flow_work_list.push({ ir->parent, jump->m_false_target });
                }
            } break;
            case BOTTOM:
                // std::println("\t\t*CondJump - BOTTOM: selecting both branches");
                flow_work_list.push({ ir->parent, jump->m_true_target });
                flow_work_list.push({ ir->parent, jump->m_false_target });
                break;
            case TOP:
                unimplemented("no idea what to do");
                break;
            }
        } else if (auto jump = dynamic_cast<jl::ir::Jump*>(ir)) {
            // std::println("\t\t*Jump");
            flow_work_list.push({ ir->parent, jump->m_target });
        } else if (auto def = ir->def()) {

            if (lattice_values[*def].type == BOTTOM) {
                // Nothing much to do since we have already seen that this variable could hold
                // any/multiple values during runtime.
                return;
            }

            auto visitor = IRVisitorForMeet(lattice_values, lattice_values[*def]);
            visitor.fold(ir);
            auto new_lattice = visitor.value;
            auto original_value = lattice_values[*ir->def()];

            // std::println("\t\t-Considering {}", ir->to_str());
            // std::println("\t\t-Original: {}, new: {}", original_value.to_str(), new_lattice.to_str());

            // if the computed value is different from the current value
            // TODO replace wiht overloaded ==
            if (original_value.type != new_lattice.type || original_value.value != new_lattice.value) {
                lattice_values[*def] = new_lattice;
                add_uses_to_ssa_worklist(*def, ir->parent);
            }
        }
    }

    bool is_edge_executed(jl::BasicBlock* start, jl::BasicBlock* end)
    {
        if (exec_map.contains({ start, end })) {
            return exec_map[{ start, end }];
        } else {
            std::stack<jl::BasicBlock*> stk;
            std::unordered_set<jl::BasicBlock*> visited;

            for (auto& [edge, flag] : exec_map) {
                if (!flag)
                    continue;

                auto [a, b] = edge;
                if (a == start) {
                    stk.push(b);
                }
            }

            while (!stk.empty()) {
                auto node = stk.top();
                stk.pop();

                if (node == end) {
                    exec_map[{ start, end }] = true;
                    return true;
                }

                if (visited.contains(node)) {
                    continue;
                }

                visited.insert(node);

                for (auto& [edge, flag] : exec_map) {
                    if (!flag)
                        continue;

                    auto [a, b] = edge;
                    if (a == node) {
                        stk.push(b);
                    }
                }
            }

            return false;
        }
    }

    // Initializes all the definitions with `TOP` and literal values
    // with `CONSTANT`
    ValueMap init_lattice_values(jl::Function* function)
    {
        ValueMap lattice_values;

        for (auto arg : function->args()) {
            lattice_values[arg] = {
                .type = BOTTOM,
                .value = std::nullopt
            };
        }

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

            const auto [left, right] = jl::algorithms::get_successors(node);

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

    // Remove unexecuted blocks and all reference to them from the CFG
    void remove_unexecuted_blocks()
    {
        std::vector<jl::BasicBlock*> blocks_to_be_deleted;
        std::unordered_map<jl::BasicBlock*, uint32_t> in_edges;

        for (auto& block : function->blocks()) {
            in_edges[block.get()] = 0;
        }

        for (auto [edge, flag] : exec_map) {
            if (flag) {
                in_edges[edge.second] += 1;
            }
        }

        auto predecessors = jl::algorithms::get_predecessors(function);

        for (auto [block, val] : in_edges) {
            if (val > 0) {
                continue;
            }

            // Mark the block to be deleted later
            blocks_to_be_deleted.push_back(block);

            // If an predecessors of the block have a conditional jump to this block,
            // then change it a unconditional jump and remove the reference this to block
            for (auto pred : predecessors[block]) {
                auto terminator = pred->get_terminator();

                if (auto jump = dynamic_cast<jl::ir::CondJump*>(terminator)) {
                    auto [succ1, succ2] = jl::algorithms::get_successors(pred);
                    auto remaining_target = jump->m_true_target == block ? jump->m_false_target : jump->m_true_target;
                    assert(remaining_target != nullptr && "atleast one live target to jump to");
                    function->remove_ir(jump);
                    function->set_current_block(pred);
                    function->add_ir(jl::ir::Jump(remaining_target, jump->m_line));
                }
            }

            // If this block is being used by a phi node, then remove the block from its
            // list of operands
            for (auto& blk : function->blocks()) {
                for (auto phi : blk->phis) {
                    auto iter = std::find_if(phi->m_opers.begin(),
                        phi->m_opers.end(),
                        [&block](auto&& pair) { return pair.second == block; });

                    if (iter != phi->m_opers.end()) {
                        phi->m_opers.erase(iter);
                    }
                }
            }
        }

        for (auto block : blocks_to_be_deleted) {
            // std::println("Deleting {}", block->get_name());
            function->remove_block(block);
        }
    }

    // Removes all defs that have a CONSTAT lattice value since
    // we now know what its value is
    std::unordered_map<jl::value::Variable, jl::ir::IR*> remove_constant_defs()
    {
        std::unordered_map<jl::value::Variable, jl::ir::IR*> to_be_removed;

        constexpr auto is_used = [](jl::Function* function, jl::value::Variable def) {
            for (auto& ir : function->irs()) {
                if (ir->is_used(def)) {
                    return true;
                }
            }

            return false;
        };

        for (auto& ir : function->irs()) {
            if (auto def = ir->def()) {

                if (lattice_values[*def].type != CONSTANT) {
                    if (is_used(function, *def)) {
                        continue;
                    }
                }

                to_be_removed[*def] = ir.get();
            }
        }

        return to_be_removed;
    }

    // Add a constant literal for all uses which has a constant value
    void add_used_constants_as_literals(std::unordered_map<jl::value::Variable, jl::ir::IR*>& to_be_removed)
    {
        class IRVisitorForUses : public jl::ir::IRVisitor {
            const ValueMap& lattice_values;
            jl::Function* function;
            std::unordered_set<uint32_t> added;
            std::unordered_map<jl::value::Variable, jl::ir::IR*>& to_be_removed;

        public:
            IRVisitorForUses(const ValueMap& lattice_values,
                jl::Function* function,
                std::unordered_map<jl::value::Variable, jl::ir::IR*>& to_be_removed)
                : lattice_values(lattice_values)
                , function(function)
                , to_be_removed(to_be_removed)
            {
            }

            std::vector<jl::ir::InitLiteral> new_literals;

        private:
            // If this variable has a constant value then add it as constant literal
            // in our CFG
            void add_literal_ir_if_constant(const jl::value::Variable& var)
            {
                // Add this only if its a constant and has not already been added
                if (lattice_values.at(var).type == CONSTANT && !added.contains(var.id())) {
                    // If the variable is already marked for removal then remove it from the list
                    // Check if its a constant literal, if its not then replace it with one
                    if (to_be_removed.contains(var)) {
                        auto ir = to_be_removed[var];
                        if (!dynamic_cast<jl::ir::InitLiteral*>(ir)) {
                            // This is not constant literal so we replace this ir with a constant literal
                            auto literal = jl::LiteralValue(*lattice_values.at(var).value);
                            auto init_literal = new jl::ir::InitLiteral(std::move(literal), var, 0);
                            function->irs().emplace_back(init_literal);
                            function->replace_ir(ir, init_literal);
                        }
                        // Already present, but marked for deletion, so remove it from the to_be_removed list
                        to_be_removed.erase(var);
                    } else {
                        // Add as a new literal
                        auto literal = jl::LiteralValue(*lattice_values.at(var).value);
                        new_literals.emplace_back(std::move(literal), var, 0);
                    }
                    added.insert(var.id());
                }
            }

            void visit_binary_ir(jl::ir::Binary& binary)
            {
                add_literal_ir_if_constant(binary.m_operand_a);
                add_literal_ir_if_constant(binary.m_operand_b);
            }

            void visit_return_ir(jl::ir::Return& ret)
            {
                if (ret.m_ret_val) {
                    add_literal_ir_if_constant(*ret.m_ret_val);
                }
            }

            void visit_call_ir(jl::ir::Call& call)
            {
                for (auto var : call.m_args) {
                    add_literal_ir_if_constant(var);
                }
            }

            void visit_cond_jump_ir(jl::ir::CondJump& jump)
            {
                add_literal_ir_if_constant(jump.m_condition);
            }

            void visit_unary_ir(jl::ir::Unary& unary)
            {
                add_literal_ir_if_constant(unary.m_dest);
            }

            void visit_allocate_list_ir(jl::ir::AllocateList& allocate)
            {
                add_literal_ir_if_constant(allocate.m_list);
                add_literal_ir_if_constant(allocate.m_fat_ptr);
            }

            void visit_allocate_var_ir(jl::ir::AllocateVar& allocate)
            {
                add_literal_ir_if_constant(allocate.m_addr);
            }

            void visit_read_ir(jl::ir::Read& read)
            {
                add_literal_ir_if_constant(read.m_base);
                if (read.m_offset) {
                    add_literal_ir_if_constant(*read.m_offset);
                }
            }

            void visit_write_ir(jl::ir::Write& write)
            {
                add_literal_ir_if_constant(write.m_base);
                if (write.m_offset) {
                    add_literal_ir_if_constant(*write.m_offset);
                }
            }

            void visit_debug_print_ir(jl::ir::DebugPrint& print)
            {
                add_literal_ir_if_constant(print.m_val);
            }

            void visit_type_cast_ir(jl::ir::TypeCast& type_cast)
            {
                add_literal_ir_if_constant(type_cast.m_source);
            }

            void visit_phi(jl::ir::Phi& phi)
            {
                for (auto [var, block] : phi.m_opers) {
                    add_literal_ir_if_constant(var);
                }
            }

            void visit_move_ir(jl::ir::Move& move) { }
            void visit_jump_ir(jl::ir::Jump& jump) { }
            void visit_label_ir(jl::ir::Label& label) { }
            void visit_init_literal_ir(jl::ir::InitLiteral& literal) { }
        };

        IRVisitorForUses visitor(lattice_values, function, to_be_removed);

        for (auto& ir : function->irs()) {

            if (auto def = ir->def()) {
                // Only consider those instructions which have not been marked for removal
                if (to_be_removed.contains(*def)) {
                    continue;
                }
            }

            ir->accept(visitor);
        }

        auto entry = function->entry_block();
        auto terminator = entry->get_terminator();
        function->set_current_block(entry);

        for (auto literal : visitor.new_literals) {
            auto var = literal.m_dest;

            // If the variable is already marked for removal then remove it from the list
            // Check if its a constant literal, if its not then replace it with one
            if (to_be_removed.contains(var)) {
                auto ir = to_be_removed[var];
                if (!dynamic_cast<jl::ir::InitLiteral*>(ir)) {
                    // This is not constant literal so we replace this ir with a constant literal
                    auto literal = jl::LiteralValue(*lattice_values.at(var).value);
                    auto init_literal = new jl::ir::InitLiteral(std::move(literal), var, 0);
                    function->irs().emplace_back(init_literal);
                    function->replace_ir(ir, init_literal);
                }
                to_be_removed.erase(var);
            } else {
                // Add as a new literal
                function->add_ir_to_front(std::move(literal));
            }
        }

        for (auto [_, ir] : to_be_removed) {
            // std::println("Removing ir: {}", ir->to_str());
            function->remove_ir(ir);
        }
    }

    void collapse_empty_blocks()
    {
        auto predecessors = jl::algorithms::get_predecessors(function);
        std::unordered_set<jl::BasicBlock*> to_be_removed;
        std::unordered_set<jl::BasicBlock*> visited;
        std::stack<jl::BasicBlock*> stk;
        stk.push(function->entry_block());

        while (!stk.empty()) {
            auto block = stk.top();
            stk.pop();

            if (visited.contains(block)) {
                continue;
            }

            visited.insert(block);

            size_t instr_count = block->phis.size();
            for (auto ir = block->head; ir != nullptr; ir = ir->next) {
                instr_count += 1;
            }

            auto terminator = block->get_terminator();

            // add the blocks to consider next
            if (auto jmp = dynamic_cast<jl::ir::Jump*>(terminator)) {
                stk.push(jmp->m_target);
            } else if (auto cjmp = dynamic_cast<jl::ir::CondJump*>(terminator)) {
                stk.push(cjmp->m_true_target);
                stk.push(cjmp->m_false_target);
            }

            // If the block has more than 1 instruction then it should not be removed
            if (instr_count > 1) {
                continue;
            }

            // if the only remaining instruction is conditional jump or return, then
            // it should not be removed
            auto next_jump = dynamic_cast<jl::ir::Jump*>(terminator);
            if (next_jump == nullptr) {
                continue;
            }

            // THis block only contains an unconditional jump, so we can safely
            // remove it
            to_be_removed.insert(block);

            // Replace all references to this block from its predecessors
            for (auto pred : predecessors[block]) {
                if (to_be_removed.contains(pred)) {
                    continue;
                }

                auto terminator = pred->get_terminator();

                if (auto cjmp = dynamic_cast<jl::ir::CondJump*>(terminator)) {
                    if (cjmp->m_true_target == block) {
                        cjmp->m_true_target = next_jump->m_target;
                    }
                    if (cjmp->m_false_target == block) {
                        cjmp->m_false_target = next_jump->m_target;
                    }
                } else if (auto jmp = dynamic_cast<jl::ir::Jump*>(terminator)) {
                    jmp->m_target = next_jump->m_target;
                }
            }
        }

        for (auto block : to_be_removed) {
            // std::println("Removing block: {}", block->get_name());
            function->remove_block(block);
        }
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

            auto [start, dest] = edge;

            if (state.is_edge_executed(start, dest)) {
                continue;
            }

            state.exec_map[edge] = true;
            // std::println("Marking edge {} -> {}", start->get_name(), dest->get_name());
            // std::println("Evaluating block: {}", dest->get_name());

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

    // std::println("Final Lattice Values:");
    for (const auto [var, val] : state.lattice_values) {
        // std::println("{} -> {}", var.to_str(), val.to_str());
    }

    // std::println("\nFinal ExecMap Values:");
    for (const auto [edge, flag] : state.exec_map) {
        // std::println("{} -> {}: {}", edge.first->get_name(), edge.second->get_name(), flag);
    }

    state.remove_unexecuted_blocks();
    auto to_be_removed = state.remove_constant_defs();
    state.add_used_constants_as_literals(to_be_removed);
    state.collapse_empty_blocks();
}
