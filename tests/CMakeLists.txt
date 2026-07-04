include(AddCatch2)

# add_executable(interpreter_tests
# interpreter/TestMain.cpp
# interpreter/TestLexer.cpp
# interpreter/TestParser.cpp
# interpreter/TestInterpreter.cpp
# )

# target_link_libraries(interpreter_tests PRIVATE
# Catch2::Catch2WithMain
# JuneInterpreter
# )

# add_executable(codegen_tests
# codegen/TestCodeExec.cpp
# codegen/TestFailing.cpp
# codegen/TestCompilation.cpp
# )

# target_link_libraries(codegen_tests PRIVATE
# Catch2::Catch2WithMain
# JuneInterpreter
# )
add_executable(opt_tests
    opt/TestDominanceTree.cpp
    opt/TestDominanceFrontier.cpp
    opt/TestMem2reg.cpp
    opt/TestSCCP.cpp
)

target_link_libraries(opt_tests PRIVATE
    Catch2::Catch2WithMain
    JuneCompiler
)

include(CTest)
include(Catch)

# catch_discover_tests(interpreter_tests)
# catch_discover_tests(codegen_tests)
catch_discover_tests(opt_tests)
