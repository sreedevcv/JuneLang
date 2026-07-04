#include "ErrorHandler.hpp"
#include "Function.hpp"
#include "IRGen_v2.hpp"
#include "Lexer.hpp"
#include "LiteralValue.hpp"
#include "Parser.hpp"
#include "SemanticAnalysis.hpp"
#include "ir/InitLiteral.hpp"
#include "ir/Return.hpp"
#include "opt/Optimizer.hpp"
#include "types/TypeContext.hpp"
#include "value/Variable.hpp"
#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <optional>
#include <print>
#include <string>
#include <unordered_map>

void transform_to_ir(const char* src, jl::LiteralValue val)
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
    jl::opt::sccp(test);

    std::unordered_map<uint32_t, jl::LiteralValue> values;

    for (auto& blk : test->blocks()) {
        for (auto ir = blk->head; ir != nullptr; ir = ir->next) {
            if (auto ret = dynamic_cast<jl::ir::Return*>(ir)) {
                if (ret->m_ret_val) {
                    REQUIRE(values.at(ret->m_ret_val->id()).data == val.data);
                }
            } else if (auto init = dynamic_cast<jl::ir::InitLiteral*>(ir)) {
                values.emplace(init->m_dest.id(), init->m_source);
            }
        }
    }
}

TEST_CASE("sccp - Diamond with merge", "sccp")
{
    transform_to_ir(R"(
fun test(): int [
    var sum = 10;

    if (sum < 3) [
        sum = 3;
    ] else [
        sum = 12 * 8;
    ]

    return sum;
]   
    )",
        jl::LiteralValue(96));
}

TEST_CASE("sccp - Basic constant folding", "sccp")
{
    transform_to_ir(R"(
fun test(): int [
    var a = 7;
    var b = 13;
    var c = a * b - 4;
    return c;
]
    )",
        jl::LiteralValue(87));
}

TEST_CASE("sccp - Float constant folding", "sccp")
{
    transform_to_ir(R"(
fun test(): float [
    var x = 2.5;
    var y = 4.0;
    var z = x * y + 1.5;
    return z;
]
    )",
        jl::LiteralValue(11.5));
}

TEST_CASE("sccp - Bool comparison", "sccp")
{
    transform_to_ir(R"(
fun test(): bool [
    var a = 15;
    var b = 20;
    return a < b;
]
    )",
        jl::LiteralValue(true));
}

TEST_CASE("sccp - If else with constant condition", "sccp")
{
    transform_to_ir(R"(
fun test(): int [
    var result = 0;
    if (10 > 100) [
        result = 999;
    ] else [
        result = 55;
    ]
    return result;
]
    )",
        jl::LiteralValue(55));
}

TEST_CASE("sccp - Diamond with merge (equal branches)", "sccp")
{
    transform_to_ir(R"(
fun test(): int [
    var n = 7;
    var x = 0;
    if (n > 0) [
        x = 100 - 58;
    ] else [
        x = 21 * 2;
    ]
    return x;
]
    )",
        jl::LiteralValue(42));
}

TEST_CASE("sccp - Nested if else chain", "sccp")
{
    transform_to_ir(R"(
fun test(): int [
    var score = 73;
    var grade = 0;
    if (score >= 90) [
        grade = 4;
    ] else [
        if (score >= 80) [
            grade = 3;
        ] else [
            if (score >= 70) [
                grade = 2;
            ] else [
                grade = 1;
            ]
        ]
    ]
    return grade;
]
    )",
        jl::LiteralValue(2));
}


TEST_CASE("sccp - Early return dead code", "sccp")
{
    transform_to_ir(R"(
fun test(): int [
    var x = 42;
    if (x == 42) [
        return 100;
    ]
    return 999;
]
    )",
        jl::LiteralValue(100));
}

TEST_CASE("sccp - Boolean logic combination", "sccp")
{
    transform_to_ir(R"(
fun test(): bool [
    var a = 5;
    var b = 10;
    var c = 15;
    return (a < b) and (b < c);
]
    )",
        jl::LiteralValue(true));
}

TEST_CASE("sccp - Negative number arithmetic", "sccp")
{
    transform_to_ir(R"(
fun test(): int [
    var a = -15;
    var b = 7;
    var c = a + b * -2;
    return c;
]
    )",
        jl::LiteralValue(-29));
}

TEST_CASE("sccp - Float threshold comparison", "sccp")
{
    transform_to_ir(R"(
fun test(): bool [
    var price = 19.99;
    var discount = 5.0;
    var final_price = price - discount;
    return final_price < 15.0;
]
    )",
        jl::LiteralValue(true));
}

