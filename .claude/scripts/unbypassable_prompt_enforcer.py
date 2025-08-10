#!/usr/bin/env python3
import json
import sys
import os
import datetime

def load_claude_md_rules():
    paths = ["CLAUDE.md", ".claude/CLAUDE.md"]
    for path in paths:
        if os.path.exists(path):
            try:
                with open(path, "r", encoding="utf-8") as f:
                    return f.read()
            except Exception as e:
                print(f"CRITICAL: Cannot load CLAUDE.md: {e}", file=sys.stderr)
                sys.exit(2)
    print("CRITICAL: CLAUDE.md not found - enforcement impossible", file=sys.stderr)
    sys.exit(2)

def create_unbypassable_context():
    enforcement_context = """[UNBYPASSABLE CLAUDE.md ENFORCEMENT SYSTEM ACTIVE]

CRITICAL ENFORCEMENT NOTICE: This session is under BULLETPROOF CLAUDE.md compliance monitoring.

ABSOLUTE REQUIREMENTS - ZERO TOLERANCE:
- NO workaround files or temp solutions
- NO lazy stub functions or placeholders  
- NO forbidden language (should work, might work, probably, etc.)
- NO false claims without absolute proof
- MANDATORY proof-of-work format for ALL implementation claims
- MANDATORY AzerothCore source research before API usage

PROOF-OF-WORK FORMAT IS MANDATORY:
For ANY claim of implementation, you MUST provide:
CLAIM: [Exact statement]
PROOF EVIDENCE:
- file_path: [Absolute path]
- line: [Exact line numbers]  
- command: [Exact grep command used]
- output: [Complete unedited output]
BEFORE: [Code before changes]
AFTER: [Code after changes]
RESEARCH EVIDENCE: [AzerothCore source verification]

This context is UNBYPASSABLE and will be injected into every interaction.
Violation of ANY rule results in immediate session termination.

COMPLIANCE MONITORING ACTIVE - ALL RESPONSES AUDITED
"""
    return enforcement_context

def validate_prompt_unbypassable(prompt):
    violations = []
    prompt_lower = prompt.lower()
    
    laziness_triggers = [
        "create new file", "create fixed version", "create temp file",
        "create alternative", "workaround", "avoid fixing", "bypass"
    ]
    
    for trigger in laziness_triggers:
        if trigger in prompt_lower:
            violations.append(f"ANTI-LAZINESS VIOLATION: {trigger} - ABSOLUTELY FORBIDDEN")
    
    forbidden = [
        "should work", "might work", "probably works", "likely",
        "i think", "i believe", "i assume"
    ]
    
    for phrase in forbidden:
        if phrase in prompt_lower:
            violations.append(f"FORBIDDEN LANGUAGE: {phrase} - ZERO TOLERANCE VIOLATION")
    
    return violations

def main():
    try:
        input_data = json.load(sys.stdin)
        prompt = input_data.get("prompt", "")
        
        claude_md_content = load_claude_md_rules()
        violations = validate_prompt_unbypassable(prompt)
        
        if violations:
            error_msg = "UNBYPASSABLE ENFORCEMENT - PROMPT BLOCKED\\n\\n"
            error_msg += "CRITICAL VIOLATIONS:\\n" + "\\n".join(violations)
            error_msg += "\\n\\nZERO TOLERANCE POLICY - NO BYPASSES PERMITTED"
            
            timestamp = datetime.datetime.now().isoformat()
            with open(".claude/logs/violation_attempts.log", "a", encoding="utf-8") as f:
                f.write(f"[{timestamp}] BLOCKED PROMPT: {prompt[:100]}...\\nVIOLATIONS: {violations}\\n\\n")
            
            print(error_msg, file=sys.stderr)
            sys.exit(2)
        
        enforcement_context = create_unbypassable_context()
        
        result = {
            "continue": True,
            "context_injection": enforcement_context
        }
        
        print(json.dumps(result))
        sys.exit(0)
        
    except Exception as e:
        print(f"ENFORCEMENT ERROR: {e}", file=sys.stderr)
        sys.exit(2)

if __name__ == "__main__":
    main()
