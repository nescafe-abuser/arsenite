Arsenite (Arc)

Arsenite is a high-performance systems programming language designed for clarity, efficiency, and seamless integration with low-level systems. By compiling directly to C Intermediate Representation (IR), Arsenite leverages the industry-leading optimization and code-generation capabilities of the Clang/LLVM toolchain.

Core Principles

Efficiency by Design: A lightweight compiler architecture that prioritizes speed and minimal overhead.

Backend Leverage: Utilizes Clang as a sophisticated backend, ensuring high-quality machine code generation and robust cross-platform support.

Explicit Typology: A strict, expressive type system (e.g., u64, i32, []string) that maps directly to C's stdint for predictable memory behavior.

Interoperability: Automated inclusion of standard system headers (stdint.h, stdio.h, stdbool.h) for immediate access to system-level primitives.

Technical Architecture

The compiler is built with a modular pipeline to ensure maintainability and high performance:

Lexical Analysis: A hand-rolled lexer that transforms raw source into a precise token stream.

Recursive Descent Parsing: Constructs a robust Abstract Syntax Tree (AST) representing the program's structural logic.

C IR Generation: Translates the AST into human-readable C code, optimized for the final compilation stage.

Toolchain Integration: Orchestrates the backend compiler (Clang) to produce binaries or LLVM bitcode.

Project Structure

.
├── src/            # Compiler entry point (compiler.cpp)
├── lib/            # Core engine (Lexer, Parser, Codegen)
├── build/          # Build artifacts and binaries (Automated)
└── examples/       # Sample Arsenite source files (.at)


Getting Started

Prerequisites

g++ (for building the compiler)

clang (as the default backend)

make

Installation

To build the compiler and organize the workspace:

make


To install the arc binary globally:

sudo make install


Usage Guide

The Arsenite compiler provides a flexible interface for different stages of the development lifecycle.

1. Compile to Executable

The standard command to produce a production-ready binary:

./build/arc main.at -o my_app


2. Output C Source Code (--nocc)

For developers who wish to inspect the generated C IR without invoking the backend:

./build/arc main.at --nocc


3. Emit LLVM IR (--ir)

Generate human-readable LLVM bitcode for deep analysis or custom optimization passes:

./build/arc main.at --ir -o logic.ll


Language Syntax at a Glance

Variables and Procedures

Arsenite uses a clear, proc-based syntax. The compiler automatically handles the mapping of the entry point to standard C requirements.

proc main(argc: i32, argv: []string) -> i32 {
    let x: u32 = 1024;
    let y: u32 = 2;
    let result: u32;

    result = x * y;

    return 0;
}


Maintenance

To remove all build artifacts and clean the environment:

make clean

