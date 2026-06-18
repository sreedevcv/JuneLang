#include "Function.hpp"
#include "ir/ConditionalJump.hpp"
#include "ir/Jump.hpp"
#include "ir/Return.hpp"
#include "types/Type.hpp"
#include "utils/algorithms.hpp"
#include "value/Variable.hpp"
#include <catch2/catch_test_macros.hpp>
#include <optional>

TEST_CASE("Dominance Tree", "DT")
{
    // entry
    //  / \
    // T   F

    auto temp_type = new jl::type::Builtin(jl::type::Builtin::Primitive::BOOL);
    jl::Function function("test1", temp_type);

    auto entry_block = function.new_block("test block");
    auto true_block = function.new_block("true block");
    auto false_block = function.new_block("false block");

    function.set_current_block(entry_block);
    function.add_ir(jl::ir::CondJump(jl::value::Variable(function.m_var_count++, temp_type), true_block, false_block, 0));

    // auto a =jl::ir::Jump(true_block, 0);
    // auto b = jl::ir::Return(std::nullopt, 0);

    auto df = jl::algorithms::dominance_tree(&function);

    REQUIRE(df[entry_block] == entry_block);
    REQUIRE(df[true_block] == entry_block);
    REQUIRE(df[false_block] == entry_block);
}

TEST_CASE("Dominance Tree - Diamond with merge", "DT")
{
    // entry
    //  / \
    // T   F
    //  \ /
    //  merge
    auto temp_type = new jl::type::Builtin(jl::type::Builtin::Primitive::BOOL);
    jl::Function function("test_diamond", temp_type);

    auto entry_block = function.new_block("entry block");
    auto true_block = function.new_block("true block");
    auto false_block = function.new_block("false block");
    auto merge_block = function.new_block("merge block");

    function.set_current_block(entry_block);
    function.add_ir(jl::ir::CondJump(jl::value::Variable(function.m_var_count++, temp_type), true_block, false_block, 0));

    function.set_current_block(true_block);
    function.add_ir(jl::ir::Jump(merge_block, 0));

    function.set_current_block(false_block);
    function.add_ir(jl::ir::Jump(merge_block, 0));

    function.set_current_block(merge_block);
    function.add_ir(jl::ir::Return(std::nullopt, 0));

    auto df = jl::algorithms::dominance_tree(&function);

    REQUIRE(df[entry_block] == entry_block);
    REQUIRE(df[true_block] == entry_block);
    REQUIRE(df[false_block] == entry_block);
    REQUIRE(df[merge_block] == entry_block);
}

TEST_CASE("Dominance Tree - Linear chain", "DT")
{
    // entry
    //   |
    //   B1
    //   |
    //   B2
    //   |
    //   B3
    auto temp_type = new jl::type::Builtin(jl::type::Builtin::Primitive::BOOL);
    jl::Function function("test_linear", temp_type);

    auto entry_block = function.new_block("entry block");
    auto b1 = function.new_block("b1 block");
    auto b2 = function.new_block("b2 block");
    auto b3 = function.new_block("b3 block");

    function.set_current_block(entry_block);
    function.add_ir(jl::ir::Jump(b1, 0));

    function.set_current_block(b1);
    function.add_ir(jl::ir::Jump(b2, 0));

    function.set_current_block(b2);
    function.add_ir(jl::ir::Jump(b3, 0));

    function.set_current_block(b3);
    function.add_ir(jl::ir::Return(std::nullopt, 0));

    auto df = jl::algorithms::dominance_tree(&function);

    REQUIRE(df[entry_block] == entry_block);
    REQUIRE(df[b1] == entry_block);
    REQUIRE(df[b2] == b1);
    REQUIRE(df[b3] == b2);
}

TEST_CASE("Dominance Tree - Diamond with early return", "DT")
{
    // entry
    //  / \
    // T   F
    // |    \
    // ret  merge
    //         |
    //        ret
    auto temp_type = new jl::type::Builtin(jl::type::Builtin::Primitive::BOOL);
    jl::Function function("test_early_return", temp_type);

    auto entry_block = function.new_block("entry block");
    auto true_block = function.new_block("true block");
    auto false_block = function.new_block("false block");
    auto merge_block = function.new_block("merge block");

    function.set_current_block(entry_block);
    function.add_ir(jl::ir::CondJump(jl::value::Variable(function.m_var_count++, temp_type), true_block, false_block, 0));

    function.set_current_block(true_block);
    function.add_ir(jl::ir::Return(std::nullopt, 0));

    function.set_current_block(false_block);
    function.add_ir(jl::ir::Jump(merge_block, 0));

    function.set_current_block(merge_block);
    function.add_ir(jl::ir::Return(std::nullopt, 0));

    auto df = jl::algorithms::dominance_tree(&function);

    REQUIRE(df[entry_block] == entry_block);
    REQUIRE(df[true_block] == entry_block);
    REQUIRE(df[false_block] == entry_block);
    // merge_block is only reachable through false_block, so false_block dominates it
    REQUIRE(df[merge_block] == false_block);
}

TEST_CASE("Dominance Tree - Nested branches", "DT")
{
    //        entry
    //        /   \
    //      T1     F1
    //     /  \      \
    //   T2    F2    merge_outer
    //     \  /         |
    //   merge_inner    ret
    //        |
    //       ret
    auto temp_type = new jl::type::Builtin(jl::type::Builtin::Primitive::BOOL);
    jl::Function function("test_nested", temp_type);

    auto entry_block = function.new_block("entry block");
    auto t1_block = function.new_block("t1 block");
    auto f1_block = function.new_block("f1 block");
    auto t2_block = function.new_block("t2 block");
    auto f2_block = function.new_block("f2 block");
    auto merge_inner_block = function.new_block("merge inner block");
    auto merge_outer_block = function.new_block("merge outer block");

    function.set_current_block(entry_block);
    function.add_ir(jl::ir::CondJump(jl::value::Variable(function.m_var_count++, temp_type), t1_block, f1_block, 0));

    function.set_current_block(t1_block);
    function.add_ir(jl::ir::CondJump(jl::value::Variable(function.m_var_count++, temp_type), t2_block, f2_block, 0));

    function.set_current_block(f1_block);
    function.add_ir(jl::ir::Jump(merge_outer_block, 0));

    function.set_current_block(t2_block);
    function.add_ir(jl::ir::Jump(merge_inner_block, 0));

    function.set_current_block(f2_block);
    function.add_ir(jl::ir::Jump(merge_inner_block, 0));

    function.set_current_block(merge_inner_block);
    function.add_ir(jl::ir::Return(std::nullopt, 0));

    function.set_current_block(merge_outer_block);
    function.add_ir(jl::ir::Return(std::nullopt, 0));

    auto df = jl::algorithms::dominance_tree(&function);

    REQUIRE(df[entry_block] == entry_block);
    REQUIRE(df[t1_block] == entry_block);
    REQUIRE(df[f1_block] == entry_block);
    REQUIRE(df[t2_block] == t1_block);
    REQUIRE(df[f2_block] == t1_block);
    // merge_inner_block reachable only via t1_block's subtree (t2 or f2), so t1_block dominates it
    REQUIRE(df[merge_inner_block] == t1_block);
    REQUIRE(df[merge_outer_block] == f1_block);
}

TEST_CASE("Dominance Tree - Simple loop", "DT")
{
    //   entry
    //     |
    //   header <---+
    //    /  \       |
    //  body   exit  |
    //    \___________+ (back edge to header)
    auto temp_type = new jl::type::Builtin(jl::type::Builtin::Primitive::BOOL);
    jl::Function function("test_loop", temp_type);

    auto entry_block = function.new_block("entry block");
    auto header_block = function.new_block("header block");
    auto body_block = function.new_block("body block");
    auto exit_block = function.new_block("exit block");

    function.set_current_block(entry_block);
    function.add_ir(jl::ir::Jump(header_block, 0));

    function.set_current_block(header_block);
    function.add_ir(jl::ir::CondJump(jl::value::Variable(function.m_var_count++, temp_type), body_block, exit_block, 0));

    function.set_current_block(body_block);
    function.add_ir(jl::ir::Jump(header_block, 0)); // back edge

    function.set_current_block(exit_block);
    function.add_ir(jl::ir::Return(std::nullopt, 0));

    auto df = jl::algorithms::dominance_tree(&function);

    REQUIRE(df[entry_block] == entry_block);
    REQUIRE(df[header_block] == entry_block);
    REQUIRE(df[body_block] == header_block);
    REQUIRE(df[exit_block] == header_block);
}

TEST_CASE("Dominance Tree - Loop with branch inside body", "DT")
{
    //    entry
    //      |
    //    header <-------------+
    //     /  \                |
    //   body  exit             |
    //   / \                    |
    // B_T  B_F                 |
    //   \  /                   |
    //  body_merge --------------+ (back edge to header)
    auto temp_type = new jl::type::Builtin(jl::type::Builtin::Primitive::BOOL);
    jl::Function function("test_loop_branch", temp_type);

    auto entry_block = function.new_block("entry block");
    auto header_block = function.new_block("header block");
    auto body_block = function.new_block("body block");
    auto body_true_block = function.new_block("body true block");
    auto body_false_block = function.new_block("body false block");
    auto body_merge_block = function.new_block("body merge block");
    auto exit_block = function.new_block("exit block");

    function.set_current_block(entry_block);
    function.add_ir(jl::ir::Jump(header_block, 0));

    function.set_current_block(header_block);
    function.add_ir(jl::ir::CondJump(jl::value::Variable(function.m_var_count++, temp_type), body_block, exit_block, 0));

    function.set_current_block(body_block);
    function.add_ir(jl::ir::CondJump(jl::value::Variable(function.m_var_count++, temp_type), body_true_block, body_false_block, 0));

    function.set_current_block(body_true_block);
    function.add_ir(jl::ir::Jump(body_merge_block, 0));

    function.set_current_block(body_false_block);
    function.add_ir(jl::ir::Jump(body_merge_block, 0));

    function.set_current_block(body_merge_block);
    function.add_ir(jl::ir::Jump(header_block, 0)); // back edge

    function.set_current_block(exit_block);
    function.add_ir(jl::ir::Return(std::nullopt, 0));

    auto df = jl::algorithms::dominance_tree(&function);

    REQUIRE(df[entry_block] == entry_block);
    REQUIRE(df[header_block] == entry_block);
    REQUIRE(df[body_block] == header_block);
    REQUIRE(df[exit_block] == header_block);
    REQUIRE(df[body_true_block] == body_block);
    REQUIRE(df[body_false_block] == body_block);
    REQUIRE(df[body_merge_block] == body_block);
}

TEST_CASE("Dominance Tree - Multiple independent branches with common sink", "DT")
{
    //          entry
    //         /     \
    //       T1        F1
    //      /  \       /  \
    //    T2    F2   T3    F3
    //      \  /       \  /
    //      mid1       mid2
    //         \        /
    //           sink
    //             |
    //            ret
    auto temp_type = new jl::type::Builtin(jl::type::Builtin::Primitive::BOOL);
    jl::Function function("test_multi_branch_sink", temp_type);

    auto entry_block = function.new_block("entry block");
    auto t1_block = function.new_block("t1 block");
    auto f1_block = function.new_block("f1 block");
    auto t2_block = function.new_block("t2 block");
    auto f2_block = function.new_block("f2 block");
    auto t3_block = function.new_block("t3 block");
    auto f3_block = function.new_block("f3 block");
    auto mid1_block = function.new_block("mid1 block");
    auto mid2_block = function.new_block("mid2 block");
    auto sink_block = function.new_block("sink block");

    function.set_current_block(entry_block);
    function.add_ir(jl::ir::CondJump(jl::value::Variable(function.m_var_count++, temp_type), t1_block, f1_block, 0));

    function.set_current_block(t1_block);
    function.add_ir(jl::ir::CondJump(jl::value::Variable(function.m_var_count++, temp_type), t2_block, f2_block, 0));

    function.set_current_block(f1_block);
    function.add_ir(jl::ir::CondJump(jl::value::Variable(function.m_var_count++, temp_type), t3_block, f3_block, 0));

    function.set_current_block(t2_block);
    function.add_ir(jl::ir::Jump(mid1_block, 0));

    function.set_current_block(f2_block);
    function.add_ir(jl::ir::Jump(mid1_block, 0));

    function.set_current_block(t3_block);
    function.add_ir(jl::ir::Jump(mid2_block, 0));

    function.set_current_block(f3_block);
    function.add_ir(jl::ir::Jump(mid2_block, 0));

    function.set_current_block(mid1_block);
    function.add_ir(jl::ir::Jump(sink_block, 0));

    function.set_current_block(mid2_block);
    function.add_ir(jl::ir::Jump(sink_block, 0));

    function.set_current_block(sink_block);
    function.add_ir(jl::ir::Return(std::nullopt, 0));

    auto df = jl::algorithms::dominance_tree(&function);

    REQUIRE(df[entry_block] == entry_block);
    REQUIRE(df[t1_block] == entry_block);
    REQUIRE(df[f1_block] == entry_block);
    REQUIRE(df[t2_block] == t1_block);
    REQUIRE(df[f2_block] == t1_block);
    REQUIRE(df[t3_block] == f1_block);
    REQUIRE(df[f3_block] == f1_block);
    REQUIRE(df[mid1_block] == t1_block);
    REQUIRE(df[mid2_block] == f1_block);
    // sink_block reachable via both mid1 (under t1) and mid2 (under f1), so only entry dominates it
    REQUIRE(df[sink_block] == entry_block);
}