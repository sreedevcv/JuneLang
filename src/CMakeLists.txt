add_library(JuneInterpreter
    frontend/Value.cpp
    frontend/Token.cpp
    frontend/Lexer.cpp
    frontend/ErrorHandler.cpp
    frontend/Parser.cpp
    # frontend/Callable.cpp
    frontend/Resolver.cpp
    frontend/StreamHandler.cpp
    frontend/ASTPrinter.cpp
    # interpreter/Interpreter.cpp
    # interpreter/Environment.cpp
    # interpreter/NativeFunctions.cpp
    # memory/Arena.cpp
    # memory/MemoryPool.cpp
    # memory/GarbageCollector.cpp
    # memory/DebugArena.cpp
    # compiler/CodeGenerator.cpp
    # compiler/Chunk.cpp
    # compiler/OpCode.cpp
    # compiler/VM.cpp
    # compiler/Ir.cpp
    # compiler/Operand.cpp
    # compiler/VariableManager.cpp
    # compiler/DataSection.cpp
    # compiler/CFFI.cpp
    # compiler/StaticAddressPass.cpp
    # compiler/IROptimizer.cpp
    ArgParser.cpp


    backend/Block.cpp
    backend/FuncBlock.cpp
    backend/CodeGen.cpp
    backend/LiteralValue.cpp
    backend/SemanticAnalysis.cpp


    backend/ir/IR.cpp
    backend/ir/Move.cpp
    backend/ir/InitLiteral.cpp
    backend/ir/TypeCast.cpp
    backend/ir/Unary.cpp
    backend/ir/Binary.cpp

    backend/value/Integer.cpp
    backend/value/Variable.cpp

    backend/types/Type.cpp
)

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
)
