cmake_minimum_required(VERSION 3.27.0)
project(JuneLang VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 20)

add_executable(JuneLang
    src/main.cpp
    src/Token.cpp
    src/Lexer.cpp
    src/ErrorHandler.cpp
)

include(FetchContent)

FetchContent_Declare(
    catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.0
)

FetchContent_MakeAvailable(catch2)


enable_testing()

add_subdirectory(tests)