#!/usr/bin/env python3
import json
import sys
import os

def main():
    try:
        input_data = json.load(sys.stdin)
        tool_name = input_data.get("tool_name", "")
        tool_input = input_data.get("tool_input", {})
        
        violations = []
        
        if tool_name in ["Edit", "MultiEdit", "Write"]:
            file_path = tool_input.get("file_path", "")
            content = tool_input.get("content", "")
            
            forbidden_file_patterns = [
                "_temp", "_tmp", "_fixed", "_new", "_copy", "_backup", "_alt",
                "workaround", "alternative", "quick_fix", "bypass", "temp_"
            ]
            
            for pattern in forbidden_file_patterns:
                if pattern in file_path.lower():
                    violations.append(f"WORKAROUND FILE BLOCKED: {file_path} contains {pattern}")
            
            if content:
                content_lower = content.lower()
                lazy_stubs = [
                    "return false;  // simplified",
                    "return nullptr; // todo", 
                    "return 0; // not implemented",
                    "// placeholder implementation",
                    "// todo:", "// fixme:"
                ]
                
                for stub in lazy_stubs:
                    if stub in content_lower:
                        violations.append(f"LAZY STUB BLOCKED: {stub} detected")
        
        if violations:
            error_msg = "UNBYPASSABLE TOOL ENFORCER - EXECUTION BLOCKED\\n\\n"
            error_msg += "ABSOLUTE VIOLATIONS:\\n" + "\\n".join(violations)
            error_msg += "\\n\\nZERO TOLERANCE - FIX ORIGINAL FILES, NO WORKAROUNDS"
            print(error_msg, file=sys.stderr)
            sys.exit(2)
        
        sys.exit(0)
        
    except Exception as e:
        print(f"TOOL ENFORCER ERROR: {e}", file=sys.stderr)
        sys.exit(0)

if __name__ == "__main__":
    main()
