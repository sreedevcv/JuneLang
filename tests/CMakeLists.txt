add_executable(tests 
    TestMain.cpp
    TestLexer.cpp
)
target_link_directories(tests PRIVATE Catch2::Catch2WithMain)

# list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(tests)