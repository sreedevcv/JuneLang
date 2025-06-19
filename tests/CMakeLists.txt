include(AddCatch2)

add_executable(interpreter_tests 
    interpreter/TestMain.cpp
    interpreter/TestLexer.cpp
    interpreter/TestParser.cpp
    interpreter/TestInterpreter.cpp
)

target_link_libraries(interpreter_tests PRIVATE 
    Catch2::Catch2WithMain
    JuneInterpreter
)

include(CTest)
include(Catch)
catch_discover_tests(interpreter_tests)