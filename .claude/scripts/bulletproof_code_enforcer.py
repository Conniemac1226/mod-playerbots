#!/usr/bin/env python3
"""BULLETPROOF CODE ENFORCER"""

import json
import sys
import os
import re

def detect_lazy_stubs(content, file_path):
    violations = []
    warnings = []
    lines = content.split('\n')
    
    for i, line in enumerate(lines, 1):
        line_strip = line.strip()
        line_lower = line_strip.lower()
        
        # Detect bare returns without validation
        if re.match(r'^\s*return\s+(false|nullptr|0|NULL)\s*;\s*$', line_strip):
            context_lines = lines[max(0, i-10):i-1]
            has_validation = any(
                'if (' in ctx_line or 'LOG_' in ctx_line
                for ctx_line in context_lines
            )
            
            if not has_validation:
                violations.append(f"LAZY STUB at line {i}: '{line_strip}' - NO VALIDATION LOGIC")
        
        # Forbidden comments
        forbidden_comments = ['simplified', 'todo', 'fixme', 'not implemented', 'placeholder', 'stub']
        if '//' in line_lower or '/*' in line_lower:
            for forbidden in forbidden_comments:
                if forbidden in line_lower:
                    violations.append(f"FORBIDDEN COMMENT at line {i}: '{forbidden}' found")
    
    return violations, warnings

def main():
    try:
        input_data = json.load(sys.stdin)
        tool_input = input_data.get('tool_input', {})
        
        file_path = tool_input.get('file_path', '')
        if not file_path:
            sys.exit(0)
        
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                content = f.read()
        except Exception:
            sys.exit(0)
        
        violations, warnings = detect_lazy_stubs(content, file_path)
        
        if violations:
            error_msg = f"BULLETPROOF CODE ENFORCER - FILE BLOCKED: {file_path}\n\n"
            error_msg += "CRITICAL VIOLATIONS:\n" + "\n".join(violations)
            error_msg += "\n\n100% IMPLEMENTATION RULE ENFORCED"
            error_msg += "\nFix ALL lazy stubs before continuing"
            print(error_msg, file=sys.stderr)
            sys.exit(1)
        
        sys.exit(0)
        
    except Exception as e:
        print(f"CODE ENFORCER ERROR: {e}", file=sys.stderr)
        sys.exit(0)

if __name__ == "__main__":
    main()