import sys
import os
sys.path.append(os.path.join(os.getcwd(), 'src', 'frontend'))

from lexer import Lexer
from parser import Parser
import json

code = """
blueprint Counter:
    has value
    has name
    
    make with own, n:
        own~>value = 0
        own~>name = n
    done
    
    does increment with own:
        own~>value = own~>value + 1
    done
    
    does decrement with own:
        when own~>value > 0:
            own~>value = own~>value - 1
        done
    done
    
    does get with own giving Num:
        forward own~>value
    done
    
    does show_info with own:
        show "Counter: |own~>name|"
        show "Value: |own~>value|"
    done
done

spec main:
    firm counter = spawn Counter with "MyCounter"
    
    < Increment >
    each i in 1..5:
        counter~>increment()
    done
    
    < Get value >
    firm val = counter~>get()
    show "Current: |val|"
    
    < Type checking >
    when val is a Num:
        firm text = val as a Text
        show "As text: |text|"
    done
    
    < Negation test >
    firm is_zero = val == 0
    when 'is_zero:
        show "Not zero!"
    done
    
    < Logic test >
    when both val > 0 and val < 100:
        show "In range!"
    done

    < Error handling >
    attempt:
        firm result = 10 / 0
    trap DivisionError:
        show "Cannot divide by zero"
    always:
        show "Cleanup"
    done
    
    counter~>show_info()
done

main()
"""

try:
    print("Lexing...")
    lexer = Lexer(code)
    # tokens = lexer.tokenize()
    # print(tokens)
    tokens = lexer.tokenize()
#     for t in tokens:
#         print(t)
    
    print("Parsing...")
    parser = Parser(tokens)
    ast = parser.parse()
    
    print("AST Generated Successfully!")
    ast_data = ast.to_dict()
    
    with open('ast.json', 'w') as f:
        json.dump(ast_data, f, indent=2)
        
    print("AST written to ast.json. Running runtime...")
    import subprocess
    result = subprocess.run(['src/runtime/NSPL.exe', 'ast.json'], capture_output=True, text=True)
    print("--- Runtime Output ---")
    print(result.stdout)
    if result.stderr:
        print("--- Runtime Error ---")
        print(result.stderr)

except Exception as e:
    print(f"Error: {e}")
    import traceback
    traceback.print_exc()
