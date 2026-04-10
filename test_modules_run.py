import sys
import os
import json
import subprocess
sys.path.append(os.path.join(os.getcwd(), 'src', 'frontend'))

from lexer import Lexer
from parser import Parser

def compile_to_json(filename, output_filename=None):
    print(f"Compiling {filename}...")
    try:
        with open(filename, 'r') as f:
            code = f.read()
            
        lexer = Lexer(code)
        tokens = lexer.tokenize()
        
        parser = Parser(tokens)
        ast = parser.parse()
        
        if not output_filename:
            output_filename = filename + ".json"
            
        with open(output_filename, 'w') as f:
            json.dump(ast.to_dict(), f, indent=2)
        print(f"Generated {output_filename}")
        return output_filename
            
    except Exception as e:
        print(f"Error compiling {filename}: {e}")
        return None

if __name__ == "__main__":
    # 1. Compile the library
    lib_json = compile_to_json("lib_math.nspl")
    if not lib_json:
        sys.exit(1)
        
    # 2. Compile the main test file
    test_json = compile_to_json("test_modules.nspl")
    if not test_json:
        sys.exit(1)
        
    # 3. Run the runtime with the test file
    print("\n--- Executing Runtime ---")
    result = subprocess.run(['src/runtime/NSPL.exe', test_json], capture_output=True, text=True)
    print(result.stdout)
    if result.stderr:
        print("Errors:")
        print(result.stderr)
