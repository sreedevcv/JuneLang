cmake_minimum_required(VERSION 3.20.0)
project(June VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 20)

# add_link_options(-fsanitize=address)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_SOURCE_DIR})

include(FetchContent)
include_directories("include")

option(BUILD_JUNE_EDITOR "Build Editor" OFF)
option(BUILD_TESTS "Build tests" OFF)

set (BUILD_JUNE_EDITOR OFF)
set (BUILD_TESTS ON)

if (BUILD_TESTS)
    enable_testing()
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
endif()

add_subdirectory(src)

if (BUILD_JUNE_EDITOR)
    add_subdirectory(editor)
    add_executable(june
        main.cpp
    )

    target_link_libraries(june PRIVATE JuneEditor JuneInterpreter glfw glm::glm)
else()
    add_executable(june
        main.cpp
    )
    target_link_libraries(june PRIVATE JuneInterpreter)
endif()

if (BUILD_TESTS)
    add_subdirectory(tests)
endif()
