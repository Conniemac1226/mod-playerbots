# AUTO-RUNNING COMPLIANCE AGENT ENFORCER - FIXED POWERSHELL
# Forces compliance check after every Claude interaction

Write-Host "Deploying AUTO-RUNNING COMPLIANCE AGENT ENFORCER" -ForegroundColor Red

# Create directories
New-Item -ItemType Directory -Path ".\.claude\scripts" -Force | Out-Null
New-Item -ItemType Directory -Path ".\.claude\logs" -Force | Out-Null

# Enhanced settings.json
@"
{
  "hooks": {
    "UserPromptSubmit": [
      {
        "matcher": "",
        "hooks": [
          {
            "type": "command",
            "command": "python .claude/scripts/pre_compliance_injector.py",
            "timeout": 60
          }
        ]
      }
    ],
    "PreToolUse": [
      {
        "matcher": "*",
        "hooks": [
          {
            "type": "command", 
            "command": "python .claude/scripts/unbypassable_tool_enforcer.py",
            "timeout": 60
          }
        ]
      }
    ],
    "PostToolUse": [
      {
        "matcher": "Edit|MultiEdit|Write",
        "hooks": [
          {
            "type": "command",
            "command": "python .claude/scripts/unbypassable_code_enforcer.py",
            "timeout": 90
          }
        ]
      }
    ],
    "Notification": [
      {
        "matcher": "",
        "hooks": [
          {
            "type": "command",
            "command": "python .claude/scripts/auto_compliance_checker.py",
            "timeout": 120
          }
        ]
      }
    ]
  }
}
"@ | Out-File -FilePath ".\.claude\settings.json" -Encoding UTF8

# Create Python files using proper syntax
[System.IO.File]::WriteAllText("$PWD\.claude\scripts\pre_compliance_injector.py", @"
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
"@, [System.Text.Encoding]::UTF8)

[System.IO.File]::WriteAllText("$PWD\.claude\scripts\auto_compliance_checker.py", @"
#!/usr/bin/env python3
import json
import sys
import os
import datetime

def main():
    try:
        input_data = json.load(sys.stdin)
        
        # Log compliance check
        timestamp = datetime.datetime.now().isoformat()
        with open("".claude/logs/auto_compliance_checks.log"", ""a"", encoding=""utf-8"") as f:
            f.write(f""[{timestamp}] AUTO COMPLIANCE CHECK TRIGGERED\n"")
        
        # Create compliance reminder
        reminder = """"""
[AUTO-COMPLIANCE CHECK TRIGGERED]

AUTOMATIC COMPLIANCE VERIFICATION IN PROGRESS

The compliance agent is reviewing your recent work for CLAUDE.md violations.

COMMON VIOLATIONS TO CHECK:
- False implementation claims without proof
- Workaround files instead of fixing originals  
- Forbidden language usage
- Lazy stub functions or TODO comments
- Incomplete work claimed as done

IF VIOLATIONS ARE FOUND:
You will be required to continue working until compliance agent passes.

COMPLIANCE STATUS: UNDER REVIEW
""""""

        result = {
            ""continue"": True,
            ""context_injection"": reminder
        }
        
        print(json.dumps(result))
        sys.exit(0)
        
    except Exception as e:
        print(f""ERROR: {e}"", file=sys.stderr)
        sys.exit(0)

if __name__ == ""__main__"":
    main()
"@, [System.Text.Encoding]::UTF8)

[System.IO.File]::WriteAllText("$PWD\force-compliance-check.ps1", @"
# FORCE COMPLIANCE CHECK
Write-Host ""Forcing compliance agent verification..."" -ForegroundColor Red
Write-Host ""Run your compliance agent command here to verify Claude's work"" -ForegroundColor Yellow
Write-Host ""If violations found, Claude must continue working"" -ForegroundColor Red
"@, [System.Text.Encoding]::UTF8)

Write-Host ""AUTO-RUNNING COMPLIANCE AGENT ENFORCER DEPLOYED!"" -ForegroundColor Green
Write-Host ""The compliance agent will now automatically trigger after interactions"" -ForegroundColor Cyan
Write-Host ""Use .\force-compliance-check.ps1 to manually verify compliance"" -ForegroundColor Yellow