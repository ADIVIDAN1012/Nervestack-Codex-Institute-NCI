# Nervestack Programming Language (NSPL)

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

Nervestack is a futuristic, **Universal User-oriented Programming (UOP)** language designed for the next era of software engineering. It integrates **Object-Oriented Programming (OOP)** with a natural, English-like syntax. This unique combination prioritizes readability and user intuition while maintaining the structural benefits of high-performance execution.

## 🚀 The Synergy: Python + C

Nervestack is designed for both **ease of use** and **raw performance**:
- **Python Frontend**: A flexible, intelligent lexer and parser written in Python handles the complex task of understanding natural language syntax and generating an Abstract Syntax Tree (AST).
- **C Runtime**: A high-performance execution engine written in C interprets the AST, providing the speed and efficiency required for demanding applications.

This dual-layered architecture ensures that you can write code as naturally as speaking, while your programs run with the efficiency of a compiled language.

## Key Features

- **Natural Language Syntax**: Keywords like `when`, `otherwise`, `traverse`, and `spec` make code readable.
- **Dynamic Typing with Type Inference**: Automatic type detection during I/O and assignments.
- **Object-Oriented Architecture**: Full support for classes (`blueprint`), inheritance (`adopt`), and members.
- **High Performance**: Powered by a dedicated C-based runtime engine.
- **Error Handling**: Structured exception handling using `attempt`, `trap`, and `conclude` blocks.

## Feature Support Status

| Feature Category     | Implementation Status | Notes                                                             |
| :------------------- | :-------------------- | :---------------------------------------------------------------- |
| **Core Syntax**      | ✅ **Stable**         | Variables, Functions, I/O, Comments, Docstrings                   |
| **Control Flow**     | ✅ **Stable**         | `when`/`otherwise`, `traverse`/`until`, `attempt`/`trap`          |
| **Data Types**       | ✅ **Stable**         | Dynamic typing (`firm`), Type Conversion, String Interpolation    |
| **Collections**      | ✅ **Stable**         | `pack`/`unpack` for data collections (v2.0)                       |
| **Object-Oriented**  | ✅ **Stable**         | Classes (`blueprint`), Single Inheritance (`adopt`), Properties   |
| **Standard Library** | ✅ **Stable**         | Basic I/O, Math, String manipulation                              |
| **Concurrency**      | ⚠️ **Experimental**   | Keywords (`paral`, `signal`) defined; runtime currently limited   |
| **Modules**          | ⚠️ **Experimental**   | Syntax (`toolkit`, `plug`) defined; file resolution in progress   |

## Installation

### Prerequisites

- **Python 3.10+**: For the frontend lexer and parser.
- **C Compiler**: GCC or Clang to build the backend runtime.
- **Git**: For version control.

### Build Instructions

1.  **Clone the Repository**

    ```bash
    git clone https://github.com/ADIVIDAN1012/Nervestack-Codex-Institute-NCI.git
    cd Nervestack-Codex-Institute-NCI
    ```

2.  **Build the Runtime Environment**

    Navigate to the C backend directory and execute the build script:

    ```bash
    cd src/runtime
    .\build.bat
    cd ../..
    ```

    This compiles the C source code into the `main.exe` interpreter (NSPL Engine).

## Usage

### 1. Create a Source File

Create a file with the `.nspl` extension, for example `hello.nspl`:

```nervestack
spec main:
    show "Hello, Nervestack!"
done

funcall main
```

### 2. Execution

**Step 1: Parse to AST**

Run the Python frontend:

```bash
python -m src.frontend.parser hello.nspl
```

**Step 2: Execute Runtime**

Run the C runtime with the generated JSON AST:

```bash
NSPL.exe hello.nspl.json
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

when value > 5:
    show "Value exceeds threshold."
otherwise:
    show "Value is within limits."
done
```

### Functions

```nervestack
spec calculate_area with radius giving Num:
    forward 3.14 * radius * radius
done

firm area = funcall calculate_area(5)
show "Area: |area|"
```

## Documentation

- [Language Specification (UOP)](docs/UOP.md)
- [Keywords Reference](docs/Keywords.md)
- [Syntax Guide](docs/Syntax.md)
- [Built-in Functions](docs/builtins.md)
- [Contributing Guide](docs/CONTRIBUTING.md)

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

Copyright © 2026 Nervestack Codex Institute (NCI).
