# Dynamic Type Conversion Test Script

## How to Test

The dynamic type conversion feature has been implemented in the C backend. To test it:

### Option 1: Using a test file

1. Navigate to `compiler_frontend_py`:

   ```powershell
   cd compiler_frontend_py
   ```

2. Create a simple `.NSPL` file that doesn't use ask in expression context (since parser doesn't support it yet)

3. Compile and run:
   ```powershell
   py frontend.py yourtest.NSPL
   cd ..\compiler_backend_c
   .\main.exe ..\ast.json
   ```

### Option 2: Manual testing

Since `ask` appears to be a statement and not an expression in the current parser, the dynamic type conversion works when ask() is called, but the value can't be directly assigned.

### What was implemented

The C backend now has `detect_and_convert_type()` that automatically converts:

- `"42"` → Number (42.0)
- `"3.14"` → Number (3.14)
- `"On"` → Boolean (true)
- `"Off"` → Boolean (false)
- `"Nil"` → Nil type
- `"hello"` → Text ("hello")

The implementation is in `main.c` lines 651-702 (detection function) and lines 1315-1333 (ask handler).

### Note

The parser currently treats `ask` as a statement keyword, not an expression. To fully utilize this feature, the parser may need to be updated to allow `ask` in expression contexts (like variable assignments).
