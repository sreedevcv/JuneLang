# JuneLang

A statically typed, compiled programming language written in C++. This project was built as a learning exercise to understand how compilers and code generation work.

The compiler targets both LLVM IR and a custom x86-64 backend. Most of the recent work has gone into the custom backend.

## Features

- Statically typed
- Primitive types: `int`, `char`, `bool`, `float`
- If/else blocks
- `while` and `for` loops
- Functions with return values
- Type inference for local variables
- LLVM backend
- Custom x86-64 backend with a linear scan register allocator

## Compiler pipeline

The compiler goes through the usual stages:

1. **Lexer and parser** — tokenize the source and build an AST.
2. **Semantic analysis** — type-check the program and resolve symbols.
3. **IR generation** — lower the AST to a custom SSA-based intermediate representation.
4. **Optimization** — run mem2reg, SCCP, and DCE on the IR.
5. **Code generation** — lower the IR to x86-64 machine instructions, allocate registers, and emit NASM-style assembly.

## Backend passes

The IR is kept in SSA form during optimization. The main passes are:

- **Mem2Reg** — promotes stack-allocated scalar locals into SSA virtual registers. It computes dominance frontiers to insert phi nodes and then renames variable uses recursively over the control-flow graph.
- **Sparse Conditional Constant Propagation (SCCP)** — propagates constants using a three-level lattice (top, constant, bottom). It maintains two worklists, one for control-flow edges and one for SSA definitions, so it can fold arithmetic and remove unreachable branches at the same time.
- **Dead Code Elimination (DCE)** — removes basic blocks that SCCP found unreachable, deletes unused definitions, materializes known constants as literals, and collapses empty unconditional-jump blocks.
- **Phi-node elimination** — lowers phi nodes back to stack-backed reads and writes so the x86 backend does not have to handle SSA directly.

## Example

```
fun fib(n: int): int [
    if (n == 0) return 0;
    if (n == 1) return 1;

    var a = 0;
    var b = 1;
    var i = 1;

    while (i < n) [
        var c = a + b;
        a = b;
        b = c;
        i = i + 1;
    ]

    return b;
]

fun factorial(n: int): int [
    var result = 1;
    for (var i = 2; i <= n; i += 1) [
        result = result * i;
    ]
    return result;
]

fun main(): int [
    return fib(10) + factorial(5);
]
```

## Prerequisites

- C++ compiler with C++23 support
- CMake 3.20+

## Building

```bash
git clone https://github.com/sreedevcv/JuneLang.git
cd JuneLang

mkdir build
cd build

cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

## Usage

```bash
./june [source_file.june]
```
