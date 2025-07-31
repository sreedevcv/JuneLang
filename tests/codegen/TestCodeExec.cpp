#include "StaticAddressPass.hpp"
#include "Utils.hpp"
#include "catch2/catch_test_macros.hpp"

#include <utility>

#include "CodeGenerator.hpp"
#include "ErrorHandler.hpp"
#include "Interpreter.hpp"
#include "Lexer.hpp"
#include "Operand.hpp"
#include "Parser.hpp"
#include "Resolver.hpp"
#include "VM.hpp"

struct CompileData {
    std::vector<jl::reg_type> temp_vars;
    jl::Chunk chunk;

    template <typename T>
    T get(const char* name) const
    {
        const auto var = chunk.get_variable_map().at(name);
        const auto& val = temp_vars[var];
        return jl::VM::get<T>(val);
    }
};

static CompileData
compile(const char* source_code)
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
    auto [chunk_map, data_section] = codegen.generate(stmts);

    REQUIRE(jl::ErrorHandler::has_error() == false);

    jl::patch_memmory_address(chunk_map, (uint64_t)data_section.data());

    jl::VM vm(chunk_map, (ptr_type)data_section.data());
    auto chunk = codegen.get_root_chunk();
    const auto [status, temp_vars] = vm.run();
    const auto var_map = chunk.get_variable_map();

    return { std::move(temp_vars), std::move(chunk) };
}

TEST_CASE("Expressions", "[Codegen]")
{
    using namespace jl;

    const auto data = compile(R"(
        var a = 10 + 2;
        var b = (a * 2) - (3 + 1);
        var c = b / 10.0;
)");

    const auto a_value  = "a";
    const auto b_value  = "b";
    const auto c_value  = "c";

    REQUIRE(data.get<int>(a_value) == 12);
    REQUIRE(data.get<int>(b_value) == 20);
    REQUIRE(data.get<double>(c_value) == 2.0);
}

TEST_CASE("While loop", "[Codegen]")
{
    using namespace jl;

    const auto data = compile(R"( 
        var i = 0;
        var sum = 0;

        while (i <= 10) [
            sum += i;
            i += 1;
        ]
)");

    const auto i  = "i";
    const auto sum  = "sum";

    REQUIRE(data.get<int>(i) == 11);
    REQUIRE(data.get<int>(sum) == 55);
}

TEST_CASE("For loop", "[Codegen]")
{
    using namespace jl;

    const auto data = compile(R"(
        var sum = 0;

        for (var i = 0; i <= 10; i += 1) [
            sum += i;
        ]
)");

    const auto sum  = "sum";

    REQUIRE(data.get<int>(sum) == 55);
}

TEST_CASE("If ladders", "[Codegen]")
{
    using namespace jl;

    const auto data = compile(R"(
        var a = 0;
        var b = 0;
        var c = 0;

        for (var i = 0; i < 3; i += 1) [            
            if (i == 0) [
                a = i;
            ] else if (i == 1) [
                b = i; 
            ] else [
                c = i; 
            ]
        ]
)");

    const auto a_value  = "a";
    const auto b_value  = "b";
    const auto c_value  = "c";

    REQUIRE(data.get<int>(a_value) == 0);
    REQUIRE(data.get<int>(b_value) == 1);
    REQUIRE(data.get<int>(c_value) == 2);
}

TEST_CASE("Simple Function", "[Codegen]")
{
    using namespace jl;

    const auto data = compile(R"(
        fun sum_till(till: int): int [
            var sum = 0;

            for (var i = 1; i <= till; i+=1) [
                sum += i;
            ]

            return sum;
        ]

        var a = sum_till(10);
)");

    const auto a_value  = "a";

    REQUIRE(data.get<int>(a_value) == 55);
}

TEST_CASE("Empty function", "[Codegen]")
{
    using namespace jl;

    const auto data = compile(R"(
        fun hai() [
        ]

        hai();
)");
}

TEST_CASE("Recursive function", "[Codegen]")
{
    using namespace jl;

    const auto data = compile(R"(
        fun factorial(n: int): int [
            if (n == 0) [
                return 1;
            ]

            return n * factorial(n - 1);
        ]

        var a = factorial(4 + 1);
)");

    const auto a_value  = "a";

    REQUIRE(data.get<int>(a_value) == 120);
}

TEST_CASE("Fibonacci function", "[Codegen]")
{
    using namespace jl;

    const auto data = compile(R"(
    fun fibonacci(n: int): int [
        var a = 0;
        var b = 1;

        if (n == 0 or n == 1) [
                return n;
        ]

        for(var i = 0; i < n; i += 1) [
                var c = a + b;
                a = b;
                b = c;
        ]

        return b;
    ]

    var f = fibonacci(6);
)");

    const auto f_value  = "f";

    REQUIRE(data.get<int>(f_value) == 13);
}

TEST_CASE("Test Charachters", "[Codegen]")
{
    using namespace jl;

    const auto data = compile(R"(
        var a = 'a';
        var b: char = '`';

        fun test_char(): char [
            return '+';
        ]

        var c = test_char();
)");

    const auto a_value  = "a";
    const auto b_value  = "b";
    const auto c_value  = "c";

    REQUIRE(data.get<char>(a_value) == 'a');
    REQUIRE(data.get<char>(b_value) == '`');
    REQUIRE(data.get<char>(c_value) == '+');
}

TEST_CASE("Index Get: Frequency Count", "[Codegen]")
{
    using namespace jl;

    const auto data = compile(R"(
        fun find_frequency(str: [char], size: int, target: char): int [
            var count = 0;

            for (var i = 0; i < size; i += 1) [
                if (str[i] == target) [
                    count += 1;
                ]
            ]

            return count;
        ]

        var str: [char] = "Malayalam";
        var size = 9;
        var t1 = 'y';
        var t2 = 'a';
        var t3 = 'z';

        var f1 = find_frequency(str, size, t1);
        var f2 = find_frequency(str, size, t2);
        var f3 = find_frequency(str, size, t3);
)");

    const auto f1_value  = "f1";
    const auto f2_value  = "f2";
    const auto f3_value  = "f3";

    REQUIRE(data.get<int>(f1_value) == 1);
    REQUIRE(data.get<int>(f2_value) == 4);
    REQUIRE(data.get<int>(f3_value) == 0);
}

TEST_CASE("Index Set: Replace Char Count", "[Codegen]")
{
    using namespace jl;

    const auto data = compile(R"(
        fun replace_char(str: [char], size: int, target: char, new_value: char): int [
            var count = 0;

            for (var i = 0; i < size; i += 1) [
                if (str[i] == target) [
                    str[i] = new_value;
                    count += 1;
                ]
            ]

            return count;
        ]

        var str = "haaai";
        var count1 = replace_char(str, 5, 'a', 'e');
        var count2 = replace_char(str, 5, 'a', 'e');

        str[2] = 'a';

        var count3 = replace_char(str, 5, 'a', 'e');
)");

    const auto count1_value  = "count1";
    const auto count2_value  = "count2";
    const auto count3_value  = "count3";

    REQUIRE(data.get<int>(count1_value) == 3);
    REQUIRE(data.get<int>(count2_value) == 0);
    REQUIRE(data.get<int>(count3_value) == 1);
}

TEST_CASE("Pointer types", "[Codegen]")
{
    using namespace jl;

    const auto data = compile(R"(
        var ia = {1, 2, 3 + 1, 4, 5};
        var ib = ia[2];
        ia[4] = -1;
        var ic = ia[4];

        var fa = {1.0, 2.1, 3.2 + 1.0, 4.5, 5.6};
        var fb = fa[2];
        fa[4] = -1.0;
        var fc = fa[4];

        var ba = {true, true, true or false, false, false};
        var bb = ba[2];
        ba[4] = true;
        var bc = ba[4];

        var ca = {'1', 'a', 'g', '@', '5'};
        var cb = ca[2];
        ca[4] = '$';
        var cc = ca[4];
)");

    const auto fb_value  = "fb";
    const auto fc_value  = "fc";

    const auto ib_value  = "ib";
    const auto ic_value  = "ic";

    const auto cb_value  = "cb";
    const auto cc_value  = "cc";

    const auto bb_value  = "bb";
    const auto bc_value  = "bc";

    REQUIRE(data.get<float_type>(fb_value) == 4.2);
    REQUIRE(data.get<float_type>(fc_value) == -1.0);

    REQUIRE(data.get<int_type>(ib_value) == 4);
    REQUIRE(data.get<int_type>(ic_value) == -1);

    REQUIRE(data.get<char>(cb_value) == 'g');
    REQUIRE(data.get<char>(cc_value) == '$');

    REQUIRE(data.get<bool>(bb_value) == true);
    REQUIRE(data.get<bool>(bc_value) == true);
}

TEST_CASE("Int and Float Pointers in functions", "[Codegen]")
{
    using namespace jl;

    const auto data = compile(R"(
        var int_list = {1, 2, 3 + 1, 4, 5};

        fun sum_int(list: [int], size: int): int [
            var s = 0;

            for (var i = 0; i < size; i += 1) [
                s += list[i];
            ]

            return s;
        ]

        var is = sum_int(int_list, 5);

        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

        var float_list = {1.0, 2.1, 3.2 + 1.0, 4.5, 5.6};

        fun sum_float(list: [float], size: int): float [
            var s = 0.0;

            for (var i = 0; i < size; i += 1) [
                s += list[i];
            ]

            return s;
        ]

        var fs = sum_float(float_list, 5);
)");

    const auto is_value  = "is";
    const auto fs_value  = "fs";

    REQUIRE(data.get<float_type>(fs_value) == 17.4);
    REQUIRE(data.get<int>(is_value) == 16);
}

TEST_CASE("Scoped variables", "[Codegen]")
{
    using namespace jl;

    const auto data = compile(R"(
        var a = 1;
        var b = 1;

        for (var i = 0; i < 10; i+=1) [
            var a = 2;
            if (a  == 1) [
                var b = 2;
            ]

            b = a;
        ]
)");

    const auto a_value  = "a";
    const auto b_value  = "b";

    REQUIRE(data.get<int>(a_value) == 1);
    REQUIRE(data.get<int>(b_value) == 2);
}

TEST_CASE("Calling Global Functions", "[Codegen]")
{
    using namespace jl;

    const auto data = compile(R"(
        fun a() [
        ]

        fun b() [
            a();
        ]

        fun c() [
            b();
        ]
)");
}

/*
TEST_CASE("QuickSort", "[Codegen]")
{
    using namespace jl;

    const auto data = compile(R"(
        var list = {5, 1, 7, 3, 9, 0, 3, 4, 2, 8, 6};

        fun is_sorted(list: [int], size: int): bool [
            for (var i = 0; i < size - 1; i += 1) [
                if (list[i] > list[i + 1]) [
                    return false;
                ]
            ]

            return true;
        ]

        fun swap(list: [int], i: int, j: int) [
            var temp = list[i];
            list[i] = list[j];
            list[j] = temp;
        ]

        fun partition(list: [int], start: int, end: int): int [
            var pivot = list[end];
            var limit = start - 1;

            for (var i = start; i <= end - 1; i += 1) [
                if (list[i] < pivot) [
                    limit += 1;
                    swap(list, i, limit);
                ]
            ]

            swap(list, limit + 1, end);

            return limit + 1;
        ]

        fun __quickSortImpl(list: [int], start: int, end: int) [
            if (start < end) [
                var pivot = partition(list, start, end);
                __quickSortImpl(list, start, pivot - 1);
                __quickSortImpl(list, pivot + 1, end);
            ]
        ]

        fun quickSort(list: [int], size: int) [
            __quickSortImpl(list, 0, size - 1);
        ]

        quickSort(list, 10);

        var min = list[0];
        var max = list[9];

        var sorted = is_sorted(list, 10);
)");

    const auto is_sorted  = "sorted";

    REQUIRE(data.get<bool>(is_sorted) == true);
}
*/

TEST_CASE("C FFI", "[Codegen]")
{
    using namespace jl;

    const auto data = compile(R"(
        extern "puts" as puts(s: [char]);
        extern "strcmp" as strCmp(s1: [char], s2: [char]): int;
        extern "atoi" as strToInt(str: [char]): int;

        puts("Hello World");

        var is_same1 = strCmp("Hello","Hai") == 0;
        var is_same2 = strCmp("Hello","Hello") == 0;
        var ten = strToInt("10");
)");

    const auto is_same1  = "is_same1";
    const auto is_same2  = "is_same2";
    const auto ten  = "ten";

    REQUIRE(data.get<bool>(is_same1) == false);
    REQUIRE(data.get<bool>(is_same2) == true);
    REQUIRE(data.get<int>(ten) == 10);
}

TEST_CASE("Typed array declaration", "[Codegen]")
{
    using namespace jl;

    const auto data = compile(R"(
        var str: [char; 20] = "Malayalam";

        var a: [int; 10];

        var b: [float; 1] = {1.0};

        var bool = {true, false, true};

        var c: [char] = {'1', 'a', 'k'};
)");
}
