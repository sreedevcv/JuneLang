include(AddCatch2)

add_executable(unit_tests 
    TestMain.cpp
    TestLexer.cpp
    TestParser.cpp
    TestInterpreter.cpp
)

target_link_libraries(unit_tests PRIVATE 
    Catch2::Catch2WithMain
    JuneInterpreter
)

include(CTest)
include(Catch)
catch_discover_tests(unit_tests)