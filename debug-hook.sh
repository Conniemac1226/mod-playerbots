#!/bin/bash
echo "🚨🚨🚨 HOOK EXECUTED! 🚨🚨🚨" 
echo "Working directory: $(pwd)"
echo "Script path: $0"
echo "Arguments: $@"
env | grep -i claude || echo "No Claude env vars"
date >> /tmp/hook-debug.log
echo "Hook called from: $(pwd)" >> /tmp/hook-debug.log