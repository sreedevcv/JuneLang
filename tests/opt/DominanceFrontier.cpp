#include "BasicBlock.hpp"
#include "Function.hpp"
#include "ir/ConditionalJump.hpp"
#include "ir/Jump.hpp"
#include "ir/Return.hpp"
#include "types/Type.hpp"
#include "utils/algorithms.hpp"
#include <catch2/catch_test_macros.hpp>
#include <print>
#include <unordered_map>
#include <unordered_set>

TEST_CASE("Dominance Frontier - Diamond with merge", "opt")
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

    auto rpo_map = jl::algorithms::RPO(function.entry_block());
    std::vector<jl::BasicBlock*> rpo(rpo_map.size(), nullptr);
    for (auto& [block, idx] : rpo_map) {
        rpo[idx] = block;
    }

    auto calculated_df = jl::algorithms::dominance_frontier(&function, rpo);

    std::unordered_map<jl::BasicBlock*, std::unordered_set<jl::BasicBlock*>> expected_df = {
        { entry_block, {} },
        { true_block, { merge_block } },
        { false_block, { merge_block } }
    };

    for (auto& [block, frontiers] : calculated_df) {
        auto& expected_frontiers = expected_df[block];

        for (auto frontier : frontiers) {
            REQUIRE(expected_frontiers.contains(frontier));
        }
    }
}

TEST_CASE("Dominance Frontier - Linear chain (no branching)", "opt")
{
    // entry -> b1 -> b2 -> exit
    auto temp_type = new jl::type::Builtin(jl::type::Builtin::Primitive::BOOL);
    jl::Function function("test_linear_chain", temp_type);

    auto entry_block = function.new_block("entry block");
    auto b1 = function.new_block("b1");
    auto b2 = function.new_block("b2");
    auto exit_block = function.new_block("exit block");

    function.set_current_block(entry_block);
    function.add_ir(jl::ir::Jump(b1, 0));

    function.set_current_block(b1);
    function.add_ir(jl::ir::Jump(b2, 0));

    function.set_current_block(b2);
    function.add_ir(jl::ir::Jump(exit_block, 0));

    function.set_current_block(exit_block);
    function.add_ir(jl::ir::Return(std::nullopt, 0));

    auto rpo_map = jl::algorithms::RPO(function.entry_block());
    std::vector<jl::BasicBlock*> rpo(rpo_map.size(), nullptr);
    for (auto& [block, idx] : rpo_map) {
        rpo[idx] = block;
    }

    auto calculated_df = jl::algorithms::dominance_frontier(&function, rpo);

    std::unordered_map<jl::BasicBlock*, std::unordered_set<jl::BasicBlock*>> expected_df = {
        { entry_block, {} },
        { b1, {} },
        { b2, {} },
        { exit_block, {} }
    };

    for (auto& [block, frontiers] : calculated_df) {
        auto& expected_frontiers = expected_df[block];

        for (auto frontier : frontiers) {
            REQUIRE(expected_frontiers.contains(frontier));
        }
    }
}

TEST_CASE("Dominance Frontier - If with no else (triangle)", "opt")
{
    //   entry
    //   /  \
    //  T    |
    //   \  /
    //   merge
    auto temp_type = new jl::type::Builtin(jl::type::Builtin::Primitive::BOOL);
    jl::Function function("test_triangle", temp_type);

    auto entry_block = function.new_block("entry block");
    auto true_block = function.new_block("true block");
    auto merge_block = function.new_block("merge block");

    function.set_current_block(entry_block);
    function.add_ir(jl::ir::CondJump(jl::value::Variable(function.m_var_count++, temp_type), true_block, merge_block, 0));

    function.set_current_block(true_block);
    function.add_ir(jl::ir::Jump(merge_block, 0));

    function.set_current_block(merge_block);
    function.add_ir(jl::ir::Return(std::nullopt, 0));

    auto rpo_map = jl::algorithms::RPO(function.entry_block());
    std::vector<jl::BasicBlock*> rpo(rpo_map.size(), nullptr);
    for (auto& [block, idx] : rpo_map) {
        rpo[idx] = block;
    }

    auto calculated_df = jl::algorithms::dominance_frontier(&function, rpo);

    std::unordered_map<jl::BasicBlock*, std::unordered_set<jl::BasicBlock*>> expected_df = {
        { entry_block, {} },
        { true_block, { merge_block } },
        { merge_block, {} }
    };

    for (auto& [block, frontiers] : calculated_df) {
        auto& expected_frontiers = expected_df[block];

        for (auto frontier : frontiers) {
            REQUIRE(expected_frontiers.contains(frontier));
        }
    }
}

TEST_CASE("Dominance Frontier - Nested diamonds", "opt")
{
    //         entry
    //         /  \
    //        A    B
    //       / \    \
    //      T1  F1   |
    //       \  /    |
    //       inner    |
    //         \     /
    //          \   /
    //          outer
    auto temp_type = new jl::type::Builtin(jl::type::Builtin::Primitive::BOOL);
    jl::Function function("test_nested_diamonds", temp_type);

    auto entry_block = function.new_block("entry block");
    auto a_block = function.new_block("A");
    auto b_block = function.new_block("B");
    auto t1_block = function.new_block("T1");
    auto f1_block = function.new_block("F1");
    auto inner_block = function.new_block("inner merge");
    auto outer_block = function.new_block("outer merge");

    function.set_current_block(entry_block);
    function.add_ir(jl::ir::CondJump(jl::value::Variable(function.m_var_count++, temp_type), a_block, b_block, 0));

    function.set_current_block(a_block);
    function.add_ir(jl::ir::CondJump(jl::value::Variable(function.m_var_count++, temp_type), t1_block, f1_block, 0));

    function.set_current_block(b_block);
    function.add_ir(jl::ir::Jump(outer_block, 0));

    function.set_current_block(t1_block);
    function.add_ir(jl::ir::Jump(inner_block, 0));

    function.set_current_block(f1_block);
    function.add_ir(jl::ir::Jump(inner_block, 0));

    function.set_current_block(inner_block);
    function.add_ir(jl::ir::Jump(outer_block, 0));

    function.set_current_block(outer_block);
    function.add_ir(jl::ir::Return(std::nullopt, 0));

    auto rpo_map = jl::algorithms::RPO(function.entry_block());
    std::vector<jl::BasicBlock*> rpo(rpo_map.size(), nullptr);
    for (auto& [block, idx] : rpo_map) {
        rpo[idx] = block;
    }

    auto calculated_df = jl::algorithms::dominance_frontier(&function, rpo);

    std::unordered_map<jl::BasicBlock*, std::unordered_set<jl::BasicBlock*>> expected_df = {
        { entry_block, {} },
        { a_block, { outer_block } },
        { b_block, { outer_block } },
        { t1_block, { inner_block } },
        { f1_block, { inner_block } },
        { inner_block, { outer_block } },
        { outer_block, {} }
    };

    for (auto& [block, frontiers] : calculated_df) {
        auto& expected_frontiers = expected_df[block];

        for (auto frontier : frontiers) {
            REQUIRE(expected_frontiers.contains(frontier));
        }
    }
}

TEST_CASE("Dominance Frontier - Loop (while-style)", "opt")
{
    //   entry
    //     |
    //     v
    //   header <----+
    //    /  \        |
    //   body  exit   |
    //    |            |
    //    +------------+
    auto temp_type = new jl::type::Builtin(jl::type::Builtin::Primitive::BOOL);
    jl::Function function("test_while_loop", temp_type);

    auto entry_block = function.new_block("entry block");
    auto header_block = function.new_block("header");
    auto body_block = function.new_block("body");
    auto exit_block = function.new_block("exit");

    function.set_current_block(entry_block);
    function.add_ir(jl::ir::Jump(header_block, 0));

    function.set_current_block(header_block);
    function.add_ir(jl::ir::CondJump(jl::value::Variable(function.m_var_count++, temp_type), body_block, exit_block, 0));

    function.set_current_block(body_block);
    function.add_ir(jl::ir::Jump(header_block, 0));

    function.set_current_block(exit_block);
    function.add_ir(jl::ir::Return(std::nullopt, 0));

    auto rpo_map = jl::algorithms::RPO(function.entry_block());
    std::vector<jl::BasicBlock*> rpo(rpo_map.size(), nullptr);
    for (auto& [block, idx] : rpo_map) {
        rpo[idx] = block;
    }

    auto calculated_df = jl::algorithms::dominance_frontier(&function, rpo);

    std::unordered_map<jl::BasicBlock*, std::unordered_set<jl::BasicBlock*>> expected_df = {
        { entry_block, {} },
        { header_block, {} },
        { body_block, { header_block } },
        { exit_block, {} }
    };

    for (auto& [block, frontiers] : calculated_df) {
        auto& expected_frontiers = expected_df[block];

        for (auto frontier : frontiers) {
            REQUIRE(expected_frontiers.contains(frontier));
        }
    }
}

TEST_CASE("Dominance Frontier - Loop with break (nested branch in body)", "opt")
{
    //     entry
    //       |
    //       v
    //     header <-------+
    //      /  \           |
    //   body   loop_exit  |
    //    / \               |
    // break  continue      |
    //   |       |          |
    //   |       +----------+
    //   |
    //   v
    //  after  <-- (also reached from loop_exit)
    auto temp_type = new jl::type::Builtin(jl::type::Builtin::Primitive::BOOL);
    jl::Function function("test_loop_with_break", temp_type);

    auto entry_block = function.new_block("entry block");
    auto header_block = function.new_block("header");
    auto body_block = function.new_block("body");
    auto loop_exit_block = function.new_block("loop exit");
    auto break_block = function.new_block("break");
    auto continue_block = function.new_block("continue");
    auto after_block = function.new_block("after");

    function.set_current_block(entry_block);
    function.add_ir(jl::ir::Jump(header_block, 0));

    function.set_current_block(header_block);
    function.add_ir(jl::ir::CondJump(jl::value::Variable(function.m_var_count++, temp_type), body_block, loop_exit_block, 0));

    function.set_current_block(body_block);
    function.add_ir(jl::ir::CondJump(jl::value::Variable(function.m_var_count++, temp_type), break_block, continue_block, 0));

    function.set_current_block(break_block);
    function.add_ir(jl::ir::Jump(after_block, 0));

    function.set_current_block(continue_block);
    function.add_ir(jl::ir::Jump(header_block, 0));

    function.set_current_block(loop_exit_block);
    function.add_ir(jl::ir::Jump(after_block, 0));

    function.set_current_block(after_block);
    function.add_ir(jl::ir::Return(std::nullopt, 0));

    auto rpo_map = jl::algorithms::RPO(function.entry_block());
    std::vector<jl::BasicBlock*> rpo(rpo_map.size(), nullptr);
    for (auto& [block, idx] : rpo_map) {
        rpo[idx] = block;
    }

    auto calculated_df = jl::algorithms::dominance_frontier(&function, rpo);

    std::unordered_map<jl::BasicBlock*, std::unordered_set<jl::BasicBlock*>> expected_df = {
        { entry_block, {} },
        { header_block, {} },
        { body_block, { header_block, after_block } },
        { loop_exit_block, { after_block } },
        { break_block, { after_block } },
        { continue_block, { header_block } },
        { after_block, {} }
    };

    for (auto& [block, frontiers] : calculated_df) {
        auto& expected_frontiers = expected_df[block];

        for (auto frontier : frontiers) {
            REQUIRE(expected_frontiers.contains(frontier));
        }
    }
}

TEST_CASE("Dominance Frontier - Diamond of diamonds (4-way merge)", "opt")
{
    //              entry
    //             /     \
    //            A       B
    //           / \     / \
    //          T1  F1  T2  F2
    //           \  /    \  /
    //          mid1     mid2
    //             \      /
    //              \    /
    //              final
    auto temp_type = new jl::type::Builtin(jl::type::Builtin::Primitive::BOOL);
    jl::Function function("test_diamond_of_diamonds", temp_type);

    auto entry_block = function.new_block("entry block");
    auto a_block = function.new_block("A");
    auto b_block = function.new_block("B");
    auto t1_block = function.new_block("T1");
    auto f1_block = function.new_block("F1");
    auto t2_block = function.new_block("T2");
    auto f2_block = function.new_block("F2");
    auto mid1_block = function.new_block("mid1");
    auto mid2_block = function.new_block("mid2");
    auto final_block = function.new_block("final");

    function.set_current_block(entry_block);
    function.add_ir(jl::ir::CondJump(jl::value::Variable(function.m_var_count++, temp_type), a_block, b_block, 0));

    function.set_current_block(a_block);
    function.add_ir(jl::ir::CondJump(jl::value::Variable(function.m_var_count++, temp_type), t1_block, f1_block, 0));

    function.set_current_block(b_block);
    function.add_ir(jl::ir::CondJump(jl::value::Variable(function.m_var_count++, temp_type), t2_block, f2_block, 0));

    function.set_current_block(t1_block);
    function.add_ir(jl::ir::Jump(mid1_block, 0));

    function.set_current_block(f1_block);
    function.add_ir(jl::ir::Jump(mid1_block, 0));

    function.set_current_block(t2_block);
    function.add_ir(jl::ir::Jump(mid2_block, 0));

    function.set_current_block(f2_block);
    function.add_ir(jl::ir::Jump(mid2_block, 0));

    function.set_current_block(mid1_block);
    function.add_ir(jl::ir::Jump(final_block, 0));

    function.set_current_block(mid2_block);
    function.add_ir(jl::ir::Jump(final_block, 0));

    function.set_current_block(final_block);
    function.add_ir(jl::ir::Return(std::nullopt, 0));

    auto rpo_map = jl::algorithms::RPO(function.entry_block());
    std::vector<jl::BasicBlock*> rpo(rpo_map.size(), nullptr);
    for (auto& [block, idx] : rpo_map) {
        rpo[idx] = block;
    }

    auto calculated_df = jl::algorithms::dominance_frontier(&function, rpo);

    std::unordered_map<jl::BasicBlock*, std::unordered_set<jl::BasicBlock*>> expected_df = {
        { entry_block, {} },
        { a_block, { final_block } },
        { b_block, { final_block } },
        { t1_block, { mid1_block } },
        { f1_block, { mid1_block } },
        { t2_block, { mid2_block } },
        { f2_block, { mid2_block } },
        { mid1_block, { final_block } },
        { mid2_block, { final_block } },
        { final_block, {} }
    };

    for (auto& [block, frontiers] : calculated_df) {
        auto& expected_frontiers = expected_df[block];

        for (auto frontier : frontiers) {
            REQUIRE(expected_frontiers.contains(frontier));
        }
    }
}

TEST_CASE("Dominance Frontier - Irreducible-ish CFG (branch into loop from two preds)", "opt")
{
    //     entry
    //     /   \
    //    A     B
    //     \   /
    //     header <----+
    //      /  \         |
    //   body   exit     |
    //     |              |
    //     +--------------+
    auto temp_type = new jl::type::Builtin(jl::type::Builtin::Primitive::BOOL);
    jl::Function function("test_two_preds_into_loop", temp_type);

    auto entry_block = function.new_block("entry block");
    auto a_block = function.new_block("A");
    auto b_block = function.new_block("B");
    auto header_block = function.new_block("header");
    auto body_block = function.new_block("body");
    auto exit_block = function.new_block("exit");

    function.set_current_block(entry_block);
    function.add_ir(jl::ir::CondJump(jl::value::Variable(function.m_var_count++, temp_type), a_block, b_block, 0));

    function.set_current_block(a_block);
    function.add_ir(jl::ir::Jump(header_block, 0));

    function.set_current_block(b_block);
    function.add_ir(jl::ir::Jump(header_block, 0));

    function.set_current_block(header_block);
    function.add_ir(jl::ir::CondJump(jl::value::Variable(function.m_var_count++, temp_type), body_block, exit_block, 0));

    function.set_current_block(body_block);
    function.add_ir(jl::ir::Jump(header_block, 0));

    function.set_current_block(exit_block);
    function.add_ir(jl::ir::Return(std::nullopt, 0));

    auto rpo_map = jl::algorithms::RPO(function.entry_block());
    std::vector<jl::BasicBlock*> rpo(rpo_map.size(), nullptr);
    for (auto& [block, idx] : rpo_map) {
        rpo[idx] = block;
    }

    auto calculated_df = jl::algorithms::dominance_frontier(&function, rpo);

    std::unordered_map<jl::BasicBlock*, std::unordered_set<jl::BasicBlock*>> expected_df = {
        { entry_block, {} },
        { a_block, { header_block } },
        { b_block, { header_block } },
        { header_block, {} },
        { body_block, { header_block } },
        { exit_block, {} }
    };

    for (auto& [block, frontiers] : calculated_df) {
        auto& expected_frontiers = expected_df[block];

        for (auto frontier : frontiers) {
            REQUIRE(expected_frontiers.contains(frontier));
        }
    }
}