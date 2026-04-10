import sys
import os
import json
sys.path.append(os.path.join(os.getcwd(), 'src', 'frontend'))

from lexer import Lexer
from parser import Parser
import subprocess

def run_nervestack_file(filename):
    print(f"--- Running {filename} ---")
    try:
        with open(filename, 'r') as f:
            code = f.read()
            
        print("Lexing...")
        lexer = Lexer(code)
        tokens = lexer.tokenize()
        
        print("Parsing...")
        parser = Parser(tokens)
        ast = parser.parse()
        
        ast_json_path = filename + ".json"
        with open(ast_json_path, 'w') as f:
            json.dump(ast.to_dict(), f, indent=2)
            
        print("Executing Runtime...")
        # Run NSPL.exe with the JSON file
        result = subprocess.run(['src/runtime/NSPL.exe', ast_json_path], capture_output=True, text=True)
        print("Output:")
        print(result.stdout)
        if result.stderr:
            print("Errors:")
            print(result.stderr)
            
    except Exception as e:
        print(f"Error running {filename}: {e}")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python run_file.py <file.nervestack>")
    else:
        run_nervestack_file(sys.argv[1])
