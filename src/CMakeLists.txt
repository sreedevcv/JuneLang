add_library(JuneInterpreter
    Value.cpp
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
    NativeFunctions.cpp
    MemoryPool.cpp
    GarbageCollector.cpp
    DebugArena.cpp
)
target_include_directories(JuneInterpreter PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})