#!/usr/bin/env python3
"""BULLETPROOF TOOL GUARDIAN"""

import json
import sys
import os

def main():
    try:
        input_data = json.load(sys.stdin)
        tool_name = input_data.get('tool_name', '')
        tool_input = input_data.get('tool_input', {})
        
        violations = []
        
        if tool_name in ['Edit', 'MultiEdit', 'Write']:
            file_path = tool_input.get('file_path', '')
            content = tool_input.get('content', '')
            
            # Anti-laziness file detection
            forbidden_patterns = [
                '_temp', '_fixed', '_new', '_copy', '_backup',
                'workaround', 'alternative', 'quick_fix'
            ]
            
            for pattern in forbidden_patterns:
                if pattern in file_path.lower():
                    violations.append(f"ANTI-LAZINESS VIOLATION: File '{file_path}' is a workaround file")
            
            # Lazy stub detection
            if content:
                content_lower = content.lower()
                lazy_patterns = [
                    'return false;  // simplified',
                    'return nullptr; // todo',
                    'return 0; // not implemented',
                    '// placeholder implementation'
                ]
                
                for pattern in lazy_patterns:
                    if pattern in content_lower:
                        violations.append(f"LAZY IMPLEMENTATION: '{pattern}' detected")
        
        if violations:
            error_msg = "BULLETPROOF TOOL GUARDIAN - EXECUTION BLOCKED\n\n"
            error_msg += "VIOLATIONS:\n" + "\n".join(violations)
            error_msg += "\n\nABSOLUTE REQUIREMENT: Fix original files, NO WORKAROUNDS"
            print(error_msg, file=sys.stderr)
            sys.exit(2)
        
        sys.exit(0)
        
    except Exception as e:
        print(f"TOOL GUARDIAN ERROR: {e}", file=sys.stderr)
        sys.exit(0)

if __name__ == "__main__":
    main()