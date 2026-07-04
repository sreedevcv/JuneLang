#include "ErrorHandler.hpp"
#include "Function.hpp"
#include "IRGen_v2.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "SemanticAnalysis.hpp"
#include "ir/AllocateVar.hpp"
#include "ir/Phi.hpp"
#include "ir/Read.hpp"
#include "ir/Return.hpp"
#include "ir/Write.hpp"
#include "opt/Optimizer.hpp"
#include "types/TypeContext.hpp"
#include "value/Variable.hpp"
#include <catch2/catch_test_macros.hpp>
#include <optional>
#include <print>
#include <string>

template <typename Checker>
void transform_to_ir(const char* src, Checker checker)
{
    std::string file_name = "test.june";

    jl::Lexer lexer(src);
    lexer.scan();

    REQUIRE(!jl::ErrorHandler::has_error());

    auto tokens = lexer.get_tokens();

    jl::Parser parser(tokens, file_name);
    auto stmts = parser.parseStatements();

    REQUIRE(!jl::ErrorHandler::has_error());

    jl::TypeContext type_context;
    jl::SemanticAnalyzer sm(file_name, type_context);

    REQUIRE(sm.type_check(stmts));

    jl::IRGenv2 cg(type_context);
    auto module = cg.generate(stmts);

    auto test = module.get_function("test");

    jl::opt::mem2reg(test);

    // Check for allocations, reads and writes

    int allocations_count = 0;
    int read_count = 0;
    int write_count = 0;

    for (auto& block : test->blocks()) {
        for (auto ir = block->head; ir != nullptr; ir = ir->next) {
            if (dynamic_cast<jl::ir::AllocateVar*>(ir)) {
                allocations_count += 1;
            }
            if (dynamic_cast<jl::ir::Read*>(ir)) {
                read_count += 1;
            }
            if (dynamic_cast<jl::ir::Write*>(ir)) {
                write_count += 1;
            }
        }
    }

    REQUIRE(allocations_count == 0);
    REQUIRE(read_count == 0);
    REQUIRE(write_count == 0);

    checker(test);
}

TEST_CASE("Mem2Reg - Diamond with merge", "mem2reg")
{
    // entry
    //  / \
    // T   F
    //  \ /
    //  merge

    auto checker = [](jl::Function* func) {
        int phi_count = 0;
        std::optional<jl::value::Variable> ret_val;
        std::optional<jl::value::Variable> phi_val;

        for (auto& block : func->blocks()) {
            for (auto ir = block->head; ir != nullptr; ir = ir->next) {
                if (auto ret = dynamic_cast<jl::ir::Return*>(ir)) {
                    ret_val = ret->m_ret_val;
                }
            }

            for (auto phi : block->phis) {
                phi_count += 1;
                phi_val = phi->m_dest;
            }
        }

        REQUIRE(phi_count == 1);
        REQUIRE(ret_val == phi_val);
    };

    transform_to_ir(R"(
fun test(n: int): int [
    var a: int = 0;

    if (n > 10) [
        a = 1;    
    ] else [
        a = 2;
    ]

    return a;
])",
        checker);
}

TEST_CASE("Mem2Reg - Fibonacci", "mem2reg")
{
    auto checker = [](jl::Function* func) {
        int phi_count = 0;
        std::optional<jl::value::Variable> ret_val;

        for (auto& block : func->blocks()) {
            for (auto ir = block->head; ir != nullptr; ir = ir->next) {
                if (auto ret = dynamic_cast<jl::ir::Return*>(ir)) {
                    ret_val = ret->m_ret_val;
                }
            }

            for (auto phi : block->phis) {
                phi_count += 1;
            }
        }

        REQUIRE(phi_count == 4);
        REQUIRE(ret_val->id() == 28);
    };

    transform_to_ir(R"(
fun test(n: int): int [
    var a = 0;
    var b = 1;
    var c = 0;
    var i = 1;

    if (n == 0) return 0;

    while (i < n) [
        c = b;
        b = a + b;
        a = c;
        i = i + 1;
    ]

    return b;
]
    
)",
        checker);
}

TEST_CASE("Mem2Reg - For loop", "mem2reg")
{
    auto checker = [](jl::Function* func) {
        int phi_count = 0;
        std::optional<jl::value::Variable> ret_val;

        for (auto& block : func->blocks()) {
            for (auto ir = block->head; ir != nullptr; ir = ir->next) {
                if (auto ret = dynamic_cast<jl::ir::Return*>(ir)) {
                    ret_val = ret->m_ret_val;
                }
            }

            for (auto phi : block->phis) {
                phi_count += 1;
            }
        }

        REQUIRE(phi_count == 2);
        REQUIRE(ret_val->id() == 17);
    };

    transform_to_ir(R"(
fun test(n: int): int [
    var sum = 0;

    for (var i = 0; i < n; i += 1) [
        sum += i;
    ]

    return sum;
]
    
)",
        checker);
}

