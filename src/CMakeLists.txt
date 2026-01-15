add_library(JuneInterpreter
    frontend/Value.cpp
    frontend/Token.cpp
    frontend/Lexer.cpp
    frontend/ErrorHandler.cpp
    frontend/Parser.cpp
    frontend/Resolver.cpp
    frontend/StreamHandler.cpp
    frontend/ASTPrinter.cpp
    frontend/SemanticAnalysis.cpp
    frontend/types/Type.cpp

    ArgParser.cpp
    backend/Block.cpp
    backend/FuncBlock.cpp
    backend/IRGen.cpp
    backend/LiteralValue.cpp


    backend/ir/IR.cpp
    backend/ir/Move.cpp
    backend/ir/InitLiteral.cpp
    backend/ir/TypeCast.cpp
    backend/ir/Unary.cpp
    backend/ir/Binary.cpp
    backend/ir/Jump.cpp
    backend/ir/Return.cpp
    backend/ir/Call.cpp
    backend/ir/DebugPrint.cpp
    backend/ir/Allocate.cpp
    backend/ir/Read.cpp
    backend/ir/Write.cpp

    backend/value/Integer.cpp
    backend/value/Variable.cpp


    backend/codegen/x86CodeGen.cpp
)

# target_compile_options(JuneInterpreter
#     PRIVATE
#     "$<$<COMPILE_LANG_AND_ID:CXX,GNU>:-fconcepts-diagnostics-depth=3>"
#     "$<$<COMPILE_LANG_AND_ID:CXX,GNU>:-fdiagnostics-all-candidates>"
# )


target_link_libraries(
    JuneInterpreter
    stdc++exp
    ffi
)

target_include_directories(JuneInterpreter
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    memory
    compiler
    interpreter
    frontend
    backend
)
