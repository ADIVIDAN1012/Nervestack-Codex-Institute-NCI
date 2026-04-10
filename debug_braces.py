
import re

def check_switch_cases(filename, start_line):
    with open(filename, 'r') as f:
        lines = f.readlines()

    # Offset to 0-indexed
    current_line_idx = start_line - 1
    
    # Fast forward to switch
    switch_found = False
    brace_depth = 0
    in_switch = False
    
    for i in range(current_line_idx, len(lines)):
        line = lines[i].strip()
        
        # Count braces in this line
        open_braces = line.count('{')
        close_braces = line.count('}')
        
        # Check for switch start
        if not in_switch:
            if 'switch (node->type)' in line:
                in_switch = True
                print(f"Switch found at line {i+1}")
            
            # Update depth tracking (function level logic)
            brace_depth += (open_braces - close_braces)
            continue
            
        # We are in switch
        previous_depth = brace_depth
        brace_depth += (open_braces - close_braces)
        
        # Check for case
        if line.startswith('case NODE_') or line.startswith('default:'):
            # When hitting a new case, we expect depth to be (Switch Depth + 1)
            # Switch was at some depth. Let's assume switch starts at Depth D.
            # Inside switch brace '{', depth is D+1.
            # So case labels should appear at depth D+1.
            # If we are deeper, previous case wasn't closed.
            
            # Note: brace_depth is current depth AFTER processing this line.
            # But 'case' is usually at the start of the block.
            # If we had content before 'case', brace_depth might be high.
            
            # Let's verify depth relative to switch start.
            # We don't know absolute depth easily without parsing from start, 
            # but we know `interpret_ast` started at line 752.
            # Let's assume interpret_ast starts with depth 0 relative to itself.
            pass
            
    # Re-doing the approach: parse from start of function.
    
    brace_stack = []
    
    # Find interpret_ast start
    start_idx = 0
    for idx, line in enumerate(lines):
        if 'Value* interpret_ast(ASTNode* node, Scope* scope) {' in line:
            start_idx = idx
            break
            
    print(f"Function starts at {start_idx + 1}")
    
    current_case = None
    switch_depth = None
    
    depth = 0
    for i in range(start_idx, len(lines)):
        line = lines[i]
        stripped = line.strip()
        
        # Calculate depth change for this line
        open_count = line.count('{')
        close_count = line.count('}')
        
        # Look for switch
        if 'switch (node->type)' in line:
            switch_depth = depth
            print(f"Switch starts at depth {switch_depth} (Line {i+1})")
        
        # Look for case
        if switch_depth is not None and (stripped.startswith('case NODE_') or stripped.startswith('default:')):
            # Expected depth at 'case' line should be switch_depth + 1
            # (Because 'switch { case ... }')
            if depth != switch_depth + 1:
                print(f"ERROR: Case '{stripped}' at line {i+1} found at depth {depth}, expected {switch_depth + 1}. Previous case unclosed?")
                return
            current_case = stripped
            # print(f"Checking case {stripped} at {i+1}")

        depth += (open_count - close_count)
        
        if depth == 0 and i > start_idx:
            print(f"Function ended at {i+1}")
            break
            
    if depth != 0:
        print(f"Parse ended with non-zero depth: {depth}")

check_switch_cases(r'c:\Users\aadit\OneDrive\Desktop\project_2_Nervestack\src\runtime\main.c', 752)
