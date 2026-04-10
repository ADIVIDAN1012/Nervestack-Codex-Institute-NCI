import sys
import os
import json
import subprocess
sys.path.append(os.getcwd())

from src.frontend.lexer import Lexer
from src.frontend.parser import Parser

def compile_and_run(filename):
    print(f"Compiling {filename}...")
    try:
        with open(filename, 'r') as f:
            code = f.read()
            
        lexer = Lexer(code)
        tokens = lexer.tokenize()
        print("Tokens generated.")
        
        parser = Parser(tokens)
        ast = parser.parse()
        print("AST parsed.")
        
        json_file = filename + ".json"
        with open(json_file, 'w') as f:
            json.dump(ast.to_dict(), f, indent=2)
            
        print(f"Running {json_file}...")
        result = subprocess.run(['src/runtime/NSPL.exe', json_file], capture_output=True, text=True)
        print(result.stdout)
        if result.stderr:
            print("Errors:", result.stderr)
            
    except Exception as e:
        print(f"Error: {e}")
        import traceback
        traceback.print_exc()

if __name__ == "__main__":
    compile_and_run("test_traverse.nspl")
