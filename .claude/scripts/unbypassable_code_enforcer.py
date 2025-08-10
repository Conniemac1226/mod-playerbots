#!/usr/bin/env python3
import json
import sys
import os
import re

def detect_all_violations(content, file_path):
    violations = []
    warnings = []
    lines = content.split("\\n")
    
    for i, line in enumerate(lines, 1):
        line_strip = line.strip()
        line_lower = line_strip.lower()
        
        if re.match(r"^\\s*return\\s+(false|nullptr|0|NULL)\\s*;\\s*$", line_strip):
            context_start = max(0, i-15)
            context_end = min(len(lines), i+5)
            context_lines = lines[context_start:context_end]
            
            validation_patterns = [
                "if (", "if(", "LOG_", "nullptr", "ERROR", "WARN",
                "check", "valid", "verify", "ensure"
            ]
            
            has_validation = any(
                any(pattern in ctx_line for pattern in validation_patterns)
                for ctx_line in context_lines
            )
            
            if not has_validation:
                violations.append(f"LAZY STUB at line {i}: {line_strip} - NO VALIDATION LOGIC")
        
        forbidden_comments = [
            "todo", "fixme", "hack", "simplified", "placeholder",
            "stub", "temporary", "not implemented", "basic", "quick"
        ]
        
        if "//" in line_lower or "/*" in line_lower or "#" in line_lower:
            for forbidden in forbidden_comments:
                if forbidden in line_lower:
                    violations.append(f"FORBIDDEN COMMENT at line {i}: {forbidden} found")
    
    return violations, warnings

def main():
    try:
        input_data = json.load(sys.stdin)
        tool_input = input_data.get("tool_input", {})
        
        file_path = tool_input.get("file_path", "")
        if not file_path:
            sys.exit(0)
        
        try:
            with open(file_path, "r", encoding="utf-8") as f:
                content = f.read()
        except Exception:
            sys.exit(0)
        
        violations, warnings = detect_all_violations(content, file_path)
        
        if violations:
            error_msg = f"UNBYPASSABLE CODE ENFORCER - FILE BLOCKED: {file_path}\\n\\n"
            error_msg += "CRITICAL CLAUDE.md VIOLATIONS:\\n" + "\\n".join(violations)
            error_msg += "\\n\\n100% IMPLEMENTATION RULE ENFORCED"
            error_msg += "\\nFIX ALL VIOLATIONS BEFORE CONTINUING"
            
            with open(".claude/logs/code_violations.log", "a", encoding="utf-8") as f:
                f.write(f"FILE BLOCKED: {file_path}\\nVIOLATIONS: {violations}\\n\\n")
            
            print(error_msg, file=sys.stderr)
            sys.exit(1)
        
        sys.exit(0)
        
    except Exception as e:
        print(f"CODE ENFORCER ERROR: {e}", file=sys.stderr)
        sys.exit(0)

if __name__ == "__main__":
    main()
