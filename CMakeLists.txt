cmake_minimum_required(VERSION 3.20.0)
project(June VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 23)

# add_compile_options(-fsanitize=address)
# add_link_options(-fsanitize=address)

include(FetchContent)

option(BUILD_TESTS "Build tests" ON)
option(BUILD_EDITOR "Build Editor" OFF)

if (BUILD_TESTS)
    enable_testing()
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
endif()

add_subdirectory(src)

add_executable(june
    main.cpp
)

if (BUILD_EDITOR)
    add_subdirectory(editor)
    target_link_libraries(june PRIVATE JuneInterpreter JuneEditor)
else()
    target_link_libraries(june PRIVATE JuneInterpreter)
endif()

if (BUILD_TESTS)
    add_subdirectory(tests)
endif()