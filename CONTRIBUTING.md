# Contributing to Nervestack

Thank you for your interest in contributing to Nervestack! This document contains technical details for developers working on the compiler and toolchain.

## 🏗️ Project Structure

```
Nervestack-Code-Engine-NCI/
├── src/
│   ├── frontend/         # Python Compiler (Lexer/Parser/AST)
│   └── runtime/          # C Interpreter (VM/Backend)
├── third_party/          # Dependencies (cJSON)
├── tools/                # VS Code Extension, Scripts, Website
├── docs/                 # Documentation files
├── examples/             # Example .NSPL files
├── tests/                # Test suite
└── README.md             # Main user documentation
```

## 🛠️ Development

### Prerequisites

- **Python 3.10 or higher**
- **C Compiler** (GCC recommended for Windows, or Clang/MSVC)
- **Git**

### Building the Compiler

#### Step 1: Compile the C Backend

Navigate to the C backend directory and build:

```bash
cd src/runtime
.\build.bat
```

This generates `main.exe` - the Nervestack runtime interpreter.

#### Step 2: Compile to AST (Frontend)

The frontend is Python-based and requires no compilation, but you run it to generate the AST:

```bash
cd src/frontend
py frontend.py ..\..\examples\your_file.NSPL
```

### Testing & Verification

To verify if a feature works:

1. **Write a test:** Create a `.NSPL` file using the feature
2. **Compile:** Run `py src/frontend/frontend.py examples/yourtest.NSPL` to generate `ast.json`
3. **Execute:** Run `.\src\runtime\main.exe ast.json`
4. **Verify:** Does it produce the expected output?

See **[IMPLEMENTATION_STATUS.md](../IMPLEMENTATION_STATUS.md)** for detailed feature status and testing recommendations.

### Running Tests

```bash
cd tests
py test_runner.py
```

## VS Code Extension

To work on the extension:

1. Open the `Nervestack-vscode-extension` folder in VS Code.
2. Press `F5` to launch a new VS Code window with the extension loaded.
