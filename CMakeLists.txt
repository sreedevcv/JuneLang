cmake_minimum_required(VERSION 3.20.0)
project(June VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 23)


enable_testing()
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

add_subdirectory(src)

add_executable(JuneLang
    main.cpp
)

target_link_libraries(JuneLang PRIVATE JuneInterpreter)

add_subdirectory(tests)