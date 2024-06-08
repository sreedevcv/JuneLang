add_library(JuneInterpreter
    Token.cpp
    Lexer.cpp
    ErrorHandler.cpp
)
target_include_directories(JuneInterpreter PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})