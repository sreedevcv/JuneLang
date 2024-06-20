cmake_minimum_required(VERSION 3.20.0)
project(June VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 23)

# add_compile_options(-fsanitize=address)
# add_link_options(-fsanitize=address)

include(FetchContent)
include_directories("include")

option(BUILD_JUNE_EDITOR "Build Editor" ON)
option(BUILD_TESTS "Build tests" OFF)

if (BUILD_TESTS)
    enable_testing()
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
endif()

add_subdirectory(src)

# message(STATUS "ndsknvjksnvjfnvjkfsdnjvkdsnkvsfvnfjk ${BUILD_JUNE_EDITOR}")


if (BUILD_JUNE_EDITOR)
    add_subdirectory(editor)
    message(STATUS "glfw source dir: ${glfw_SOURCE_DIR}")
    message(STATUS "glfw binary dir: ${glfw_BINARY_DIR}")
    message(STATUS "glm source dir: ${glm_SOURCE_DIR}")
    message(STATUS "glm binary dir: ${glm_BINARY_DIR}")

    add_executable(june
        main.cpp
    )


    target_link_libraries(june PRIVATE JuneEditor glfw glm::glm)
else()
    target_link_libraries(june PRIVATE JuneInterpreter)
endif()

if (BUILD_TESTS)
    add_subdirectory(tests)
endif()