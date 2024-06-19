cmake_minimum_required(VERSION 3.20.0)
project(June VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 23)

include(FetchContent)

option(BUILD_TESTS "Build tests" OFF)

if (BUILD_TESTS)
    enable_testing()
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
endif()

add_subdirectory(src)
add_subdirectory(editor)

add_executable(june
    main.cpp
)

target_link_libraries(june PRIVATE JuneInterpreter JuneEditor)

if (BUILD_TESTS)
    add_subdirectory(tests)
endif()