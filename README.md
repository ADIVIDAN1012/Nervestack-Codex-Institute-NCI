# Nervestack Programming Language

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

Nervestack is a futuristic, **Universal User-oriented Programming (UOP)** language designed for the next era of software engineering. It integrates **Object-Oriented Programming (OOP)** with the **Universal User-Oriented Programming (UOP)** paradigm. This unique combination prioritizes readability and user intuition while maintaining the structural benefits of object-oriented design, bridging the gap between human thought processes and machine execution.

## Introduction

Nervestack is designed as a general-purpose language that emphasizes clarity. By utilizing keywords and control flow structures that mirror natural English, Nervestack attempts to make source code self-documenting and accessible, while maintaining the capabilities required for complex software development, including object-oriented design and concurrency support.

## Key Features

- **Natural Language Syntax**: Keywords and grammar are selected to approximate natural English sentences, improving readability.
- **Dynamic Typing with Type Inference**: Variables are dynamically typed (`firm`), with automatic type detection during I/O operations.
- **Object-Oriented Architecture**: Full support for classes (`blueprint`), inheritance (`adopt`), and encapsulation.
- **Concurrency Models**: Experimental support for parallel execution primitives.
- **Error Handling**: Structured exception handling using `attempt`, `trap`, and `conclude` blocks.

## Feature Support Status

The following table explicitly outlines the current implementation status of Nervestack language features.

| Feature Category     | Implementation Status | Notes                                                             |
| :------------------- | :-------------------- | :---------------------------------------------------------------- |
| **Core Syntax**      | ✅ **Stable**         | Variables, Functions, I/O, Comments, Docstrings                   |
| **Control Flow**     | ✅ **Stable**         | `check`/`alter`, `traverse`/`until`, `attempt`/`trap`             |
| **Data Types**       | ✅ **Stable**         | Dynamic typing (`firm`), Type Conversion, String Interpolation    |
| **Collections**      | ✅ **Stable**         | `pack`/`unpack` for data collections (v2.0)                       |
| **Object-Oriented**  | ✅ **Stable**         | Classes (`blueprint`), Single Inheritance (`adopt`), Properties   |
| **Standard Library** | ✅ **Stable**         | Basic I/O, Math, String manipulation                              |
| **Concurrency**      | ⚠️ **Experimental**   | Keywords (`paral`, `signal`) defined; runtime currently limited   |
| **Modules**          | ⚠️ **Experimental**   | Syntax (`toolkit`, `plug`) defined; file resolution in progress   |
| **Functional**       | 🚧 **Roadmap**        | `transform` (map), `condense` (reduce) planned for future release |

## Installation

### Prerequisites

- **Python 3.10+**: Required for the frontend lexer and parser.
- **C Compiler**: GCC (Windows/Linux) or Clang (macOS) required to build the backend runtime.
- **Git**: Version control system for cloning the repository.

### Build Instructions

1.  **Clone the Repository**

    ```bash
    git clone https://github.com/ADIVIDAN1012/Nervestack-Code-Engine-NCE.git
    cd Nervestack-Code-Engine-NCE
    ```

2.  **Build the Runtime Environment**

    Navigate to the C backend directory and execute the build script:

    ```bash
    cd src/runtime
    .\build.bat
    cd ../..
    ```

    This process compiles the C source code into the `main.exe` interpreter.

## Usage

Nervestack programs are executed in two stages: parsing (frontend) and interpretation (backend).

### 1. Create a Source File

Create a file with the `.ns` extension, for example `hello.ns`:

```nervestack
spec main:
    show "Hello, World!"
done

funcall main
```

### 2. Execution

**Step 1: Parse to AST**

Run the Python frontend to generate the Abstract Syntax Tree (AST):

```bash
python -m src.frontend.parser hello.ns
```

**Step 2: Execute Runtime**

Run the C runtime with the generated JSON AST:

```bash
NSPL.exe hello.ns.json
```

## Language Examples

### Variable Declaration and I/O

```nervestack
firm user_name = ask "Enter your name: "
show "Welcome, |user_name|."
```

### Control Flow

```nervestack
firm value = 10

check (value > 5):
    show "Value exceeds threshold."
altern:
    show "Value is within limits."
done
```

### Functions

```nervestack
spec calculate_area(radius):
    forward 3.14 * radius * radius
done

firm area = funcall calculate_area(5)
show "Area: |area|"
```

## Documentation

For detailed language specifications, refer to the following documentation:

- [Language Specification (UOP)](docs/UOP.md)
- [Keywords Reference](docs/Keywords.md)
- [Syntax and Grammar](docs/Syntax.md)
- [Standard Library](docs/builtins.md)
- [Contributing Guide](docs/CONTRIBUTING.md)

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

Copyright © 2026 Nervestack Codex Institute (NCI).
