#include "catch2/catch_test_macros.hpp"

#include "CodeGenerator.hpp"
#include "ErrorHandler.hpp"
#include "Interpreter.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "Resolver.hpp"
#include "VM.hpp"
#include <string>

jl::VM::InterpretResult compile(std::string file_name)
{
    using namespace jl;

    Lexer lexer(file_name);
    lexer.scan();

    REQUIRE(ErrorHandler::has_error() == false);

    auto tokens = lexer.get_tokens();
    Parser parser(tokens, file_name);
    auto stmts = parser.parseStatements();

    REQUIRE(ErrorHandler::has_error() == false);

    Interpreter interpreter(file_name);
    Resolver resolver(interpreter, file_name);
    resolver.resolve(stmts);

    REQUIRE(ErrorHandler::has_error() == false);

    CodeGenerator codegen(file_name);
    const auto [chunk_map, data_section] = codegen.generate(stmts);

    REQUIRE(ErrorHandler::has_error() == false);

    VM vm(chunk_map, (ptr_type)data_section.data());
    auto chunk = codegen.get_root_chunk();
    const auto [result, vars] = vm.run();
    return result;
}

TEST_CASE("Example Files", "[Execution]")
{
    const std::array<std::string, 4> file_paths = {
        "HelloWorld.june",
        "Sorting.june",
        "C.june",
        "Malloc.june",
    };

    for (const auto& path : file_paths) {
        std::string full_path = std::string { EXAMPLES_FILE_PATH "/" } + path;
        const auto result = compile(std::move(full_path));
        REQUIRE(result == jl::VM::OK);
    }
}