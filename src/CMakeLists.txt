add_library(JuneInterpreter
    Token.cpp
    Lexer.cpp
    ErrorHandler.cpp
    Parser.cpp
    Interpreter.cpp
    Environment.cpp
    Callable.cpp
    Resolver.cpp
    StreamHandler.cpp
    Arena.cpp
)
target_include_directories(JuneInterpreter PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})