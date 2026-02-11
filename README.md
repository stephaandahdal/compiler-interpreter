# Simple Programming Language Compiler & Interpreter

A complete compiler and interpreter implementation for a C-like imperative programming language, built from scratch in C++.

## 🎯 Project Overview

This project implements a full compilation pipeline including lexical analysis, parsing, intermediate representation generation, and program execution through a custom virtual machine.

## ✨ Features

- **Lexical Analysis**: Tokenizes source code into keywords, identifiers, operators, and literals
- **Recursive-Descent Parser**: Validates syntax and constructs intermediate representation
- **Symbol Table Management**: Tracks variable locations and constant values using hash maps
- **Instruction-Based VM**: Executes programs through sequential instruction processing

### Supported Language Constructs

- Variable declarations
- Arithmetic operations: `+`, `-`, `*`, `/`
- Input/Output statements
- Control flow:
  - `if` statements
  - `while` loops
  - `for` loops
  - `switch/case/default` statements
- Nested control structures

## 🛠️ Technical Details

**Languages**: C++, Bash

**Key Concepts Applied**:
- Compiler design principles
- Data structures (hash tables, linked lists, vectors)
- Recursive algorithms
- Memory management
- Control flow graph construction

## 📁 Project Structure

```
.
├── myproj3.cc         # Parser implementation (recursive-descent)
├── execute.cc         # VM executor and main entry point
├── execute.h          # Instruction types and data structures
├── lexer.cc           # Lexical analyzer implementation
├── lexer.h            # Token definitions
├── inputbuf.cc        # Input buffer utilities
├── inputbuf.h         # Input buffer interface
├── test1.sh           # Automated test runner
└── provided_tests/    # 60+ test cases
```

## 🚀 Usage

### Compilation

```bash
g++ -o a.out myproj3.cc execute.cc inputbuf.cc lexer.cc -std=c++11
```

### Running a Program

```bash
./a.out < program.txt
```

### Program Format

```
variable_list;
{
    statement_list
}
input_values
```

### Example Program

```
a, b, c, i;
{
    a = 1;
    b = 1;
    output a;
    output b;
    
    i = 3;
    WHILE i < 10 {
        c = a + b;
        output c;
        a = b;
        b = c;
        i = i + 1;
    }
}
3 2 1 4 2
```

**Output**: `1 1 2 3 5 8 13 21 34 55`

## 🧪 Testing

Run all test cases:

```bash
chmod +x test1.sh
./test1.sh
```

The test suite includes 60+ cases covering:
- Basic assignments
- Arithmetic operations
- Nested conditionals
- Loop constructs
- Switch statements
- Edge cases

## 📚 Course Context

Developed for CSE340 (Principles of Programming Languages) at Arizona State University.

## 📄 License

Copyright © Rida Bazzi, 2017-2025

Academic project - provided framework files retain original copyright.
