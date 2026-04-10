import sys
import os
sys.path.append(os.getcwd())
from src.frontend.lexer import Lexer
from src.frontend.parser import Parser

try:
    with open('test_float.nspl', 'r') as f:
        code = f.read()
    lexer = Lexer(code)
    tokens = lexer.tokenize()
    print("Tokens:", tokens)
    parser = Parser(tokens)
    print("Parsed successfully")
except Exception as e:
    print(e)
