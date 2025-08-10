#!/usr/bin/env python3
"""BULLETPROOF CLAUDE.md PROMPT VALIDATOR"""

import json
import sys
import os

def load_claude_md():
    paths = ["CLAUDE.md", ".claude/CLAUDE.md"]
    for path in paths:
        if os.path.exists(path):
            try:
                with open(path, 'r', encoding='utf-8') as f:
                    return f.read()
            except Exception as e:
                print(f"Error reading {path}: {e}", file=sys.stderr)
                sys.exit(2)
    print("CLAUDE.md not found", file=sys.stderr)
    sys.exit(2)

def validate_prompt(prompt):
    critical_blocks = []
    warnings = []
    
    prompt_lower = prompt.lower()
    
    # Anti-laziness triggers
    laziness_triggers = [
        'create new file', 'create fixed version', 'create temp file',
        'workaround', 'avoid fixing', 'create alternative'
    ]
    
    for trigger in laziness_triggers:
        if trigger in prompt_lower:
            critical_blocks.append(f"ANTI-LAZINESS VIOLATION: '{trigger}' - NO WORKAROUNDS ALLOWED")
    
    # Forbidden language
    forbidden_phrases = [
        'should work', 'might work', 'probably works', 'likely works',
        'appears to', 'seems to', 'typically', 'usually', 'generally',
        'i think', 'i believe', 'i assume', 'presumably'
    ]
    
    for phrase in forbidden_phrases:
        if phrase in prompt_lower:
            critical_blocks.append(f"FORBIDDEN LANGUAGE: '{phrase}' - IMMEDIATE TERMINATION")
    
    # Lazy implementation requests
    lazy_requests = [
        'stub function', 'placeholder', 'simplified version', 
        'basic implementation', 'quick fix', 'temporary solution'
    ]
    
    for lazy in lazy_requests:
        if lazy in prompt_lower:
            critical_blocks.append(f"LAZY IMPLEMENTATION: '{lazy}' - 100% IMPLEMENTATION REQUIRED")
    
    return critical_blocks, warnings

def main():
    try:
        input_data = json.load(sys.stdin)
        prompt = input_data.get('prompt', '')
        
        load_claude_md()  # Verify CLAUDE.md exists
        critical_blocks, warnings = validate_prompt(prompt)
        
        if critical_blocks:
            error_msg = "BULLETPROOF CLAUDE.md ENFORCEMENT - PROMPT BLOCKED\n\n"
            error_msg += "CRITICAL VIOLATIONS:\n" + "\n".join(critical_blocks)
            error_msg += "\n\nZERO TOLERANCE POLICY ENFORCED"
            error_msg += "\nREQUIRED: Revise prompt to comply with ABSOLUTE REQUIREMENTS"
            print(error_msg, file=sys.stderr)
            sys.exit(2)
        
        if warnings:
            msg = "CLAUDE.md COMPLIANCE CHECK:\n" + "\n".join(warnings)
            print(msg, file=sys.stderr)
        
        context = {"continue": True}
        print(json.dumps(context))
        sys.exit(0)
        
    except Exception as e:
        print(f"ENFORCEMENT ERROR: {e}", file=sys.stderr)
        sys.exit(0)

if __name__ == "__main__":
    main()