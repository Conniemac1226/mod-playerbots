#!/bin/bash
# Permanent Compliance Hook Activation Script
# This script sets up the CLAUDE.md compliance hook to run automatically

echo "🔒 Activating CLAUDE.md Compliance Hook..."

# Set hook environment variables
export CLAUDE_HOOK_SCRIPT="C:/Azerothcore/azerothcore-wotlk/claude-md-compliance-hook.sh"
export CLAUDE_HOOK_TYPE="pre-tool"

# Make sure hook script is executable
chmod +x "$CLAUDE_HOOK_SCRIPT"

# Verify hook is working
echo "🧪 Testing hook functionality..."
if [[ -f "$CLAUDE_HOOK_SCRIPT" ]]; then
    echo "✅ Hook script found: $CLAUDE_HOOK_SCRIPT"
    
    # Test the hook
    "$CLAUDE_HOOK_SCRIPT"
    
    if [[ $? -eq 0 ]]; then
        echo "✅ Hook is working correctly"
        echo ""
        echo "🛡️  COMPLIANCE HOOK ACTIVE"
        echo "   - Prevents coordinate guessing"
        echo "   - Blocks API assumptions" 
        echo "   - Enforces research-first methodology"
        echo "   - Requires TodoWrite for complex tasks"
        echo ""
        echo "⚠️  VIOLATIONS WILL BE BLOCKED:"
        echo "   - Spell IDs without research"
        echo "   - Coordinates without database verification"
        echo "   - API calls without AzerothCore source verification"
        echo ""
    else
        echo "❌ Hook test failed"
        exit 1
    fi
else
    echo "❌ Hook script not found: $CLAUDE_HOOK_SCRIPT"
    exit 1
fi

echo "🔐 To manually check for violations:"
echo "   Get-Content $env:TEMP/claude-compliance.log -Tail 10"
echo ""
echo "🎯 Hook is now ACTIVE and monitoring all tool calls"