from lexer import Lexer
from parser import Parser
import json
from nervestack_ast import ProgramNode
import sys

import os
import subprocess

def generate_ast_json(source_code: str):
    # Lexing
    lexer: Lexer = Lexer(source_code)
    tokens = lexer.tokenize()

    # Parsing
    parser: Parser = Parser(tokens)
    ast: ProgramNode = parser.parse()

    # Serialize AST to JSON
    ast_json = json.dumps(ast.to_dict(), indent=4)
    
    # Save AST to absolute path to avoid confusion
    ast_path = os.path.abspath('ast.json')
    with open(ast_path, 'w') as f:
        f.write(ast_json)
    print(f"AST saved to {ast_path}")

    # Execute with C++ Backend
    # Locate main.exe
    if getattr(sys, 'frozen', False):
        # Running as compiled executable
        base_path = sys._MEIPASS
        backend_exe = os.path.join(base_path, 'main.exe')
    else:
        # Running as script
        script_dir = os.path.dirname(os.path.abspath(__file__))
        backend_exe = os.path.join(script_dir, '..', 'runtime', 'main.exe')
    
    backend_exe = os.path.abspath(backend_exe)

    if os.path.exists(backend_exe):
        print("\n--- Execution Output ---")
        try:
            result = subprocess.run([backend_exe, ast_path], capture_output=True, text=True)
            print(result.stdout)
            if result.stderr:
                print("Errors:", result.stderr)
        except Exception as e:
            print(f"Failed to execute backend: {e}")
    else:
        print(f"Warning: Backend executable not found at {backend_exe}")

if __name__ == "__main__":
    if len(sys.argv) > 1:
        file_path = sys.argv[1]
        try:
            with open(file_path, 'r') as f:
                code = f.read()
            generate_ast_json(code)
        except FileNotFoundError:
            print(f"Error: File not found at {file_path}")
    else:
        print("Usage: python frontend.py <file_path>")