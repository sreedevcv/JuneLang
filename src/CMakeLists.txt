add_library(JuneCompiler
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
    frontend/types/TypeContext.cpp
    ArgParser.cpp
    backend/SemanticBlock.cpp
    backend/BasicBlock.cpp

    # backend/IRGen.cpp
    backend/LiteralValue.cpp
    backend/Module.cpp
    backend/Function.cpp
    backend/IRGen_v2.cpp
    backend/ir/IR.cpp
    backend/ir/Move.cpp
    backend/ir/InitLiteral.cpp
    backend/ir/TypeCast.cpp
    backend/ir/Unary.cpp
    backend/ir/Binary.cpp
    backend/ir/Jump.cpp
    backend/ir/ConditionalJump.cpp
    backend/ir/Return.cpp
    backend/ir/Call.cpp
    backend/ir/DebugPrint.cpp
    backend/ir/AllocateList.cpp
    backend/ir/AllocateVar.cpp
    backend/ir/Read.cpp
    backend/ir/Write.cpp
    backend/ir/Phi.cpp
    backend/value/Variable.cpp
    backend/utils/RPO.cpp
    backend/utils/DominanceTree.cpp
    backend/utils/DominanceFrontier.cpp
    backend/utils/UseIter.cpp

    # backend/codegen/x86CodeGen.cpp
    backend/opt/mem2reg.cpp
    backend/opt/sccp.cpp
    backend/opt/RemovePhiNodes.cpp
    backend/codegen/x86/Generator.cpp
    backend/codegen/x86/MachineBlock.cpp
    backend/codegen/x86/MachineFunction.cpp
    backend/codegen/x86/LivenessAnalysis.cpp

    # backend/opt/RegAlloc.cpp
    llvm_backend/JuneModule.cpp
    llvm_backend/LLVMIRGen.cpp
    llvm_backend/JuneFunction.cpp
)

# target_compile_options(JuneInterpreter
# PRIVATE
# "$<$<COMPILE_LANG_AND_ID:CXX,GNU>:-fconcepts-diagnostics-depth=3>"
# "$<$<COMPILE_LANG_AND_ID:CXX,GNU>:-fdiagnostics-all-candidates>"
# )
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")

llvm_map_components_to_libnames(LLVM_LIBS
    Core
    Support
    Target
    TargetParser
    CodeGen
    MC
    X86
)

target_link_libraries(
    JuneCompiler
    stdc++exp
    ffi
    ${LLVM_LIBS}
)

target_compile_definitions(JuneCompiler PRIVATE
    ${LLVM_DEFINITIONS}
)

target_include_directories(JuneCompiler
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    memory
    compiler
    interpreter
    frontend
    backend
    ${LLVM_INCLUDE_DIRS}
)
