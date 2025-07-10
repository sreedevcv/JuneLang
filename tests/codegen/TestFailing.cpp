#include "VM.hpp"
#include "catch2/catch_test_macros.hpp"

#include "CodeGenerator.hpp"
#include "ErrorHandler.hpp"
#include "Interpreter.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "Resolver.hpp"

void compile(const char* source_code)
{
    using namespace jl;

    std::string file_name = "test.jun";
    jl::Lexer lexer(source_code);
    lexer.scan();

    REQUIRE(jl::ErrorHandler::has_error() == false);

    auto tokens = lexer.get_tokens();
    jl::Parser parser(tokens, file_name);
    auto stmts = parser.parseStatements();

    REQUIRE(jl::ErrorHandler::has_error() == false);

    jl::Interpreter interpreter(file_name);
    jl::Resolver resolver(interpreter, file_name);
    resolver.resolve(stmts);

    REQUIRE(jl::ErrorHandler::has_error() == false);

    jl::CodeGenerator codegen(file_name);
    const auto chunk_map = codegen.generate(stmts);

    REQUIRE(jl::ErrorHandler::has_error() == true);

    // jl::VM vm;
    // const auto chunk = codegen.get_root_chunk();
    // const auto [status, temp_vars] = vm.run(chunk, chunk_map);
    // const auto var_map = chunk.get_variable_map();

    // return { temp_vars, var_map };
}

TEST_CASE("Incorrect variable store", "[Codegen Fail]")
{
    using namespace jl;

    compile(R"( 
        fun hai(): int [
            if (1 == 2) [
                var d = 3;
            ]

            return hai();
        ]
        
        var b: bool = hai();
)");
}

TEST_CASE("Storing return val of void func", "[Codegen Fail]")
{
    using namespace jl;

    compile(R"( 
        fun hai() [
            if (1 == 2) [
                var d = 3;
            ]

            return hai();
        ]
        
        var b: bool = hai();
)");
}

TEST_CASE("Incorrect arity(None)", "[Codegen Fail]")
{
    using namespace jl;

    compile(R"( 
        fun hai(a: int, b: float, c: bool): bool [
            if (1 == 2) [
                var d = 3;
            ]

            return hai();
        ]
        
        var b: bool = hai();
)");
}

TEST_CASE("Incorrect arity(Less)", "[Codegen Fail]")
{
    using namespace jl;

    compile(R"( 
        fun hai(a: int, b: float, c: bool): bool [
            if (1 == 2) [
                var d = 3;
            ]

            return hai();
        ]
        
        var b: bool = hai(5 + 1);
)");
}

TEST_CASE("Incorrect arity(More)", "[Codegen Fail]")
{
    using namespace jl;

    compile(R"( 
        fun hai(a: int, b: float, c: bool): bool [
            if (1 == 2) [
                var d = 3;
            ]

            return hai();
        ]
        
        var b: bool = hai(5 + 1, 1.2, false, 67);
)");
}

TEST_CASE("Wrong types", "[Codegen Fail]")
{
    using namespace jl;

    compile(R"( 
        fun hai(a: int, b: float, c: bool): bool [
            if (1 == 2) [
                var d = 3;
            ]

            return true;
        ]
        
        var b: bool = hai(5 + 1, 1, false);
)");
}

TEST_CASE("Calling non function", "[Codegen Fail]")
{
    using namespace jl;

    compile(R"( 
        var a = 10;
        var b = a(1, 2);
)");
}

// This will fail at  runtime
// TEST_CASE("Empty function Without return", "[Codegen]")
// {
//     using namespace jl;

//     compile(R"(
//         fun hai(): int [
//         ]

//         hai();
// )");
// }

TEST_CASE("Incorrect array size 1", "[Codegen Fail]")
{
    using namespace jl;

    compile(R"( 
        var str: [char; 0] = "Malayalam";
    )");
}


TEST_CASE("Incorrect array type 1", "[Codegen Fail]")
{
    using namespace jl;

    compile(R"( 
        var bool: [int; 3] = {true, false, true};
    )");
}
