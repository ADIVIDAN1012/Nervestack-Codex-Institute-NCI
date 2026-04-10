
def check_braces(filename):
    with open(filename, 'r') as f:
        lines = f.readlines()

    stack = []
    for i, line in enumerate(lines):
        for j, char in enumerate(line):
            if char == '{':
                stack.append((i + 1, j + 1))
            elif char == '}':
                if not stack:
                    print(f"Extra closing brace at line {i + 1}, col {j + 1}")
                    return
                stack.pop()
    
    if stack:
        print(f"Unclosed opening brace at line {stack[-1][0]}, col {stack[-1][1]}")
        # Print path for context
        for item in stack:
             print(f"  Path: Line {item[0]}")
    else:
        print("Braces are balanced.")

check_braces(r'c:\Users\aadit\OneDrive\Desktop\project_2_Nervestack\src\runtime\main.c')
