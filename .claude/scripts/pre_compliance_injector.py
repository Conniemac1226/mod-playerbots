#!/usr/bin/env python3
import json
import sys

def main():
    try:
        input_data = json.load(sys.stdin)
        
        compliance_context = """"""
[AUTO-COMPLIANCE ENFORCER ACTIVE]

WARNING: After you respond, the compliance agent will automatically run to verify your work.

MANDATORY REQUIREMENTS:
1. If you claim implementation is complete - MUST provide proof-of-work evidence
2. If you create files - MUST be fixing originals, NO workaround files allowed  
3. If you use APIs - MUST show research from AzerothCore source
4. NO forbidden language (should work, might work, probably, etc.)

THE COMPLIANCE AGENT WILL:
- Automatically check your response for violations
- Force you to continue working if violations found
- Block acceptance of incomplete work

YOU CANNOT COMPLETE UNTIL COMPLIANCE AGENT PASSES YOU.
""""""
        
        result = {
            ""continue"": True,
            ""context_injection"": compliance_context
        }
        
        print(json.dumps(result))
        sys.exit(0)
        
    except Exception as e:
        print(f""ERROR: {e}"", file=sys.stderr)
        sys.exit(0)

if __name__ == ""__main__"":
    main()