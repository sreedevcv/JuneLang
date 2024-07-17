#include <catch2/catch_test_macros.hpp>

#include <iostream>
#include <string>
#include <vector>

#include "Arena.hpp"
#include "ErrorHandler.hpp"
#include "Interpreter.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "Resolver.hpp"
#include "Token.hpp"

void test_string_with_no_error(const char* source, int arena_size = 1000)
{
    jl::Lexer lexer(source);
    jl::ErrorHandler::clear_errors();
    jl::ErrorHandler::m_stream.setOutputToFile("../../tests/scripts/temp.txt");

    std::string file_name = "test";
    lexer.scan();
    REQUIRE(jl::ErrorHandler::has_error() == false);
    auto tokens = lexer.get_tokens();

    jl::Arena arena(arena_size);
    jl::Parser parser(arena, tokens, file_name);
    auto stmts = parser.parseStatements();
    REQUIRE(jl::ErrorHandler::has_error() == false);

    jl::Interpreter interpreter(arena, file_name);
    jl::Resolver resolver(interpreter, file_name);
    resolver.resolve(stmts);

    REQUIRE(jl::ErrorHandler::has_error() == false);

    jl::Value v;
    interpreter.interpret(stmts);
    REQUIRE(jl::ErrorHandler::has_error() == false);
}

enum ErrorLoc {
    LEXER,
    PARSER,
    RESOLVER,
    INTERPRETER,
};

void test_string_with_error(const char* source, ErrorLoc errorLoc, int arena_size = 1000)
{
    jl::Lexer lexer(source);
    jl::ErrorHandler::clear_errors();
    jl::ErrorHandler::m_stream.setOutputToFile("../../tests/scripts/temp.txt");

    std::string file_name = "test";
    lexer.scan();
    auto tokens = lexer.get_tokens();
    bool has_error = jl::ErrorHandler::has_error();
    bool test_passed = (errorLoc != LEXER) ? has_error == false : has_error == true;
    REQUIRE(test_passed);
    if (has_error) {
        return;
    }

    jl::Arena arena(arena_size);
    jl::Parser parser(arena, tokens, file_name);
    auto stmts = parser.parseStatements();
    has_error = jl::ErrorHandler::has_error();
    test_passed = (errorLoc != PARSER) ? has_error == false : has_error == true;
    REQUIRE(test_passed);
    if (has_error) {
        return;
    }

    jl::Interpreter interpreter(arena, file_name, 2000*1000);
    jl::Resolver resolver(interpreter, file_name);
    resolver.resolve(stmts);
    has_error = jl::ErrorHandler::has_error();
    test_passed = (errorLoc != RESOLVER) ? has_error == false : has_error == true;
    REQUIRE(test_passed);

    if (has_error) {
        return;
    }

    jl::Value v;
    interpreter.interpret(stmts);
    has_error = jl::ErrorHandler::has_error();
    test_passed = (errorLoc != INTERPRETER) ? has_error == false : has_error == true;
    REQUIRE(test_passed);
}

TEST_CASE("Interpreter Recursive Function", "[Interpreter]")
{
    const char* source = R"(
        fun fib(n) [
            if (n <= 1) return n;
            return fib(n - 2) + fib(n - 1);
        ]

        for (var i = 0; i < 18; i += 1) [
            print fib(i);
        ]
    )";

    test_string_with_no_error(source, 1000 * 1000 * 1000);
}

TEST_CASE("Interpreter returning callback from function", "[Interpreter]")
{
    const char* source = R"(
        fun makeCounter() [
            var i = 0;

            fun count() [
                i = i + 1;
                print i;
            ]

            return count;
        ]

        var counter = makeCounter();
        counter(); // "1".
        counter(); // "2".

    )";

    test_string_with_no_error(source);
}

TEST_CASE("Interpreter Binding", "[Interpreter]")
{
    const char* source = R"(
        var a = "global";
        [
            fun showA() [
                print a;
            ]

            showA();
            var a = "block";
            showA();
        ]
    )";

    test_string_with_no_error(source);
}

TEST_CASE("Interpreter Function name printing", "[Interpreter]")
{
    const char* source = R"(
        fun add(a, b) [
            print a + b;
        ]

        print add;
    )";

    test_string_with_no_error(source);
}

TEST_CASE("Interpreter Using return statement", "[Interpreter]")
{
    const char* source = R"(
        fun count(n) [
            while (n < 100) [
                if (n == 3) return n; // <--
                print n;
                n = n + 1;
            ]
        ]

        count(1);
    )";

    test_string_with_no_error(source);
}

TEST_CASE("Interpreter returning callback from class with self", "[Interpreter]")
{
    const char* source = R"(
        class Thing [
            getCallback() [
                fun localFunction() [
                    print self;
                ]

                return localFunction;
            ]
        ]

        var callback = Thing().getCallback();
        callback();
    )";

    test_string_with_no_error(source);
}

TEST_CASE("Interpreter returning self object", "[Interpreter]")
{
    const char* source = R"(
        class Egotist [
            speak() [
                print self;
            ]
        ]

        var method = Egotist().speak;
        method();
    )";

    test_string_with_no_error(source, 2000);
}

TEST_CASE("Interpreter using self", "[Interpreter]")
{
    const char* source = R"(
        class Cake [
            taste() [
                var adjective = "delicious";
                print "The " + self.flavor + " cake is " + adjective + "!";
            ]
        ]

        var cake = Cake();
        cake.flavor = "German chocolate";
        cake.taste();
    )";

    test_string_with_no_error(source, 3000);
}

TEST_CASE("Interpreter returning bound method", "[Interpreter]")
{
    const char* source = R"(
        class Person [
            sayName() [
                print self.name;
            ]
        ]

        var jane = Person();
        jane.name = "Jane";

        var bill = Person();
        bill.name = "Bill";

        bill.sayName = jane.sayName;
        bill.sayName();
    )";

    test_string_with_no_error(source, 1000 * 1000);
}


TEST_CASE("Interpreter Using init() outside a class", "[Interpreter]")
{
    // Prints Foo instance three times
    const char* source = R"(
        class Foo [
            init() [
                print self;
            ]
        ]

        var foo = Foo();
        print foo.init();

    )";

    test_string_with_no_error(source);
}


TEST_CASE("Interpreter Simple inheritance", "[Interpreter]")
{
    const char* source = R"(
        class Doughnut [
            cook() [
                print "Fry until golden brown.";
            ]
        ]

        class BostonCream : Doughnut []

        BostonCream().cook();
    )";

    test_string_with_no_error(source);
}

TEST_CASE("Interpreter Simple inheritance with usage of super", "[Interpreter]")
{
    const char* source = R"(
        class Doughnut [
            cook() [
                print "Fry until golden brown.";
            ]
        ]

        class BostonCream : Doughnut [
            cook() [
                super.cook();
                print "Pipe full of custard and coat with chocolate.";
            ]
        ]

        BostonCream().cook();

    )";

    test_string_with_no_error(source, 2000);
}

TEST_CASE("Interpreter Calling correct method when using super", "[Interpreter]")
{
    // Should print "A method"
    const char* source = R"(
        class A [
            method() [
                print "A method";
            ]
        ]

        class B : A [
            method() [
                print "B method";
            ]

            test() [
                super.method();
            ]
        ]

        class C : B []

        C().test();
    )";

    test_string_with_no_error(source, 3000);
}

TEST_CASE("Interpreter List: Finding max element", "[Interpreter]")
{
    const char* source = R"(
        var a = {3, 5, 1, 2, 6, 9, 8, 4, 7};

        fun findMax(list) [
            var max = -1;

            for (var i = 0; i < 9; i+=1) [
                if (list[i] > max) [
                    max = list[i];
                ]
            ]

            return max;
        ]

        print findMax(a);
    )";

    test_string_with_no_error(source, 2000);
}

TEST_CASE("Interpreter Bubble sort", "[Interpreter]")
{
    const char* source = R"(
        var a = {3, 5, 1, 2, 6, 9, 8, 4, 7};

        fun bubbleSort(list, size) [
            for (var i = 0; i < size - 1; i += 1) [
                for (var j = 0; j < size - i - 1; j += 1) [
                    if (list[j] > list[j + 1]) [
                        var temp = list[j];
                        list[j] = list[j + 1];
                        list[j + 1] = temp;
                    ]
                ]
            ]
        ]

        bubbleSort(a, 9);

        for (var i = 0; i < 9; i += 1) [
            print a[i];
        ]
    )";

    test_string_with_no_error(source, 11000);
}

TEST_CASE("Interpreter Finding Mod of a list of numbers with their indices", "[Interpreter]")
{
    const char* source = R"(
        var a = {3, 5, 1, 2, 6, 9, 8, 4, 7};

        fun findMod(list) [
            for (var i = 0; i < 9; i+=1) [
                var temp = list[i] % (i + 1);
                print str(list[i]) + " % " + str(i + 1) + " is " + str(temp);
            ]

        ]

        findMod(a);
    )";

    test_string_with_no_error(source, 4000);
}

// TEST_CASE("Interpreter", "[Interpreter]")
// {
//     const char* source = R"(
//     )";

//     test_string_with_no_error(source);
// }

TEST_CASE("Interpreter Declaration with the same variable", "[Interpreter]")
{
    const char* source = R"(
        var a = "outer";
        [
            var a = a;
        ]
    )";

    test_string_with_error(source, RESOLVER);
}

TEST_CASE("Interpreter Redeclaration of same variable in scope", "[Interpreter]")
{
    const char* source = R"(
        fun bad() [
            var a = "first";
            var a = "second";
        ]
    )";

    test_string_with_error(source, RESOLVER);
}

TEST_CASE("Interpreter Return at top level", "[Interpreter]")
{
    const char* source = R"(
        return "at top level";
    )";

    test_string_with_error(source, RESOLVER);
}

TEST_CASE("Interpreter Using this at top level", "[Interpreter]")
{
    const char* source = R"(
        print self;
    )";

    test_string_with_error(source, RESOLVER);
}

TEST_CASE("Interpreter Using this inside a function", "[Interpreter]")
{
    const char* source = R"(
        fun notAMethod() [
            print self;
        ]
    )";

    test_string_with_error(source, RESOLVER);
}

TEST_CASE("Interpreter Returning explicity from a constructor", "[Interpreter]")
{
    const char* source = R"(
        class Foo [
            init() [
                return "something else";
            ]
        ]
    )";

    test_string_with_error(source, RESOLVER);
}

TEST_CASE("Interpreter Use of super inside a fucntion", "[Interpreter]")
{
    const char* source = R"(
        class Eclair [
            cook() [
                super.cook();
                print "Pipe full of crème pâtissière.";
            ]
        ]
    )";

    test_string_with_error(source, RESOLVER);
}

TEST_CASE("Interpreter Use of super at top level", "[Interpreter]")
{
    const char* source = R"(
        super.notEvenInAClass();
    )";

    test_string_with_error(source, RESOLVER);
}

// TEST_CASE("Interpreter", "[Interpreter]")
// {
//     const char* source = R"(
//     )";

//     test_string_with_error(source, RESOLVER);
// }

        

