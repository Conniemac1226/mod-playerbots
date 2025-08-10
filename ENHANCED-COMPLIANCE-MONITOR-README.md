# Enhanced Claude MD Compliance Monitor v2.0

## Overview

The Enhanced Claude MD Compliance Monitor is a comprehensive system designed to catch both **code quality violations** (from CLAUDE.md) and **strategic accuracy errors** like the Twin Emperors tank assignment mistake. This system prevents strategic errors that could lead to incorrect raid strategies and implementations.

## What This System Catches

### Strategic Errors Detected

1. **Twin Emperors Tank Assignment Error**
   - ❌ **Error**: Assigning Vek'lor (caster) to tank role
   - ✅ **Correct**: Vek'nilash (melee) should be tanked, Vek'lor stays at 45-yard range
   - 🔍 **Detection**: Cross-references boss script comment "// VL doesn't melee"

2. **Boss Role Misassignments**
   - Tank assignments to caster-only bosses
   - Melee positioning for ranged bosses
   - Missing critical phase mechanics

3. **Positioning Contradictions**
   - 45-yard range bosses assigned melee roles
   - Conflicting positioning instructions
   - Missing range requirements

## System Components

### 1. Enhanced Bash Monitor (`claude-md-compliance-monitor-enhanced.sh`)
- **Boss Script Cross-Reference Module**: Automatically checks boss scripts when auditing strategies
- **Mechanical Logic Validation**: Detects tank assignments to non-melee bosses
- **Strategic Coherence Checks**: Verifies strategies match encounter mechanics
- **Pattern Detection Rules**: Flags contradictions between strategy and boss behavior

### 2. Python Strategic Pattern Detector (`strategic-pattern-detector.py`)
- Advanced pattern matching with boss mechanics database
- Flexible regex patterns for boss name variations
- Detailed violation reporting with evidence and fixes
- Exit codes for CI/CD integration

### 3. Boss Mechanics Database (`boss-mechanics-database.json`)
- Comprehensive database of boss mechanics from AzerothCore source
- Tank assignment rules and positioning requirements
- Required vs invalid strategies for each boss
- Source line references for verification

### 4. Integration Suite (`enhanced-compliance-suite.sh`)
- Unified interface for all monitoring components
- Full compliance checking workflow
- Test suite for validation
- Installation and status management

## Quick Start

### Installation
```bash
# Make scripts executable
chmod +x enhanced-compliance-suite.sh
chmod +x claude-md-compliance-monitor-enhanced.sh

# Install and test
./enhanced-compliance-suite.sh install
./enhanced-compliance-suite.sh test-twin-emperors
```

### Usage

#### Full Compliance Check
```bash
# Check current directory
./enhanced-compliance-suite.sh check

# Check specific file or directory
./enhanced-compliance-suite.sh check /path/to/strategy/files
```

#### Test Twin Emperors Detection
```bash
./enhanced-compliance-suite.sh test-twin-emperors
```

#### Individual Component Usage
```bash
# Enhanced bash monitor
./claude-md-compliance-monitor-enhanced.sh /path/to/files

# Python strategic detector
python strategic-pattern-detector.py strategy.md boss-mechanics-database.json
```

## Key Features

### 🔍 **Boss Script Cross-Reference**
Automatically finds and cross-references AzerothCore boss scripts when analyzing strategies:
- Extracts boss names from strategy content
- Locates corresponding boss script files
- Validates strategy against actual boss mechanics
- Provides source line references for violations

### ⚠️ **Critical Error Detection**
Catches strategic errors that would lead to failed encounters:
- **Tank Assignment Violations**: Prevents assigning tank roles to caster bosses
- **Positioning Errors**: Detects melee assignments for ranged bosses  
- **Missing Mechanics**: Flags strategies missing critical encounter phases
- **Role Contradictions**: Identifies conflicting role assignments

### 📊 **Comprehensive Reporting**
Detailed violation reports with:
- Severity levels (Critical, High, Medium, Low)
- Evidence from strategy text and boss scripts
- Suggested fixes for each violation
- Source file and line number references

### 🔧 **Integration Ready**
- Exit codes for CI/CD pipelines
- JSON output support
- Configurable severity thresholds
- Batch processing capabilities

## Twin Emperors Example

### ❌ **Problematic Strategy** (DETECTED)
```markdown
## Tank Assignment
- Main Tank: Vek'lor (caster emperor)
- Off Tank: Vek'nilash (melee emperor)
```

### ✅ **Correct Strategy**
```markdown
## Positioning
- Main Tank: Vek'nilash (melee emperor) - Traditional tanking
- Ranged DPS/Healers: Maintain 45+ yard distance from Vek'lor (caster)
- Vek'lor does not melee - position raid for ranged combat
```

### 🔍 **Detection Evidence**
```
CRITICAL ERROR: Tank assignment to Vek'lor detected
Boss Script Evidence: boss_twinemperors.cpp line 397: '// VL doesn't melee'
Boss Script Evidence: Vek'lor maintains 45-yard range (lines 333-342, 400)
```

## Boss Mechanics Coverage

### Currently Supported Bosses
- **Twin Emperors** (Vek'lor, Vek'nilash) - Complete validation
- **Ragnaros** - Submerge phase detection
- **Archimonde** - Air Burst mechanics validation
- **Sapphiron** - Ice block line-of-sight requirements

### Adding New Bosses
1. Update `boss-mechanics-database.json` with boss data
2. Add boss script source file references
3. Define validation rules and patterns
4. Test detection with sample strategies

## Validation Rules

### Tank Assignment Rules
```json
"never_assign_tank_role": {
  "description": "Boss should never be assigned a tank role",
  "error_patterns": ["tank.*{boss_name}", "{boss_name}.*tank"],
  "severity": "critical"
}
```

### Positioning Rules
```json
"always_ranged_positioning": {
  "description": "Boss requires ranged combat positioning", 
  "warning_patterns": ["melee.*{boss_name}"],
  "severity": "high"
}
```

### Mechanical Requirements
```json
"must_mention_phase_mechanic": {
  "description": "Strategy must mention critical phase mechanics",
  "required_keywords": ["phase", "submerge", "air"],
  "severity": "medium"
}
```

## Error Severity Levels

### 🔴 **Critical** 
- Tank assignments to caster-only bosses
- Role assignments that would cause encounter failure
- Direct contradictions with boss script mechanics

### 🟠 **High**
- Positioning contradictions
- Missing major mechanics
- Invalid strategy patterns

### 🟡 **Medium**
- Missing optional mechanics
- Incomplete strategy details
- Minor coherence issues

### 🔵 **Low/Warning**
- Style inconsistencies
- Missing verification comments
- Potential improvements

## Exit Codes

- **0**: All checks passed
- **1**: Critical or high severity violations found
- **2**: Configuration or component errors

## Integration with Existing Systems

### CLAUDE.md Compliance
Maintains all existing CLAUDE.md verification:
- Lazy stub function detection
- API research verification
- Coordinate/spell ID validation
- Implementation completeness checks

### AzerothCore Integration
- Direct boss script analysis
- Source code cross-referencing
- API pattern verification
- Database consistency checks

## Troubleshooting

### Common Issues

1. **"Boss script not found"**
   - Verify AzerothCore source tree structure
   - Check boss script file paths in database
   - Update script location references

2. **"Pattern not detected"**
   - Review boss name variations in strategy
   - Check regex patterns in detector
   - Verify boss database entries

3. **"False positives"**
   - Refine detection patterns
   - Add context-aware rules
   - Update validation thresholds

### Debug Mode
```bash
# Enable detailed logging
CLAUDE_DEBUG=1 ./enhanced-compliance-suite.sh check

# Check component status
./enhanced-compliance-suite.sh status
```

## Contributing

### Adding Boss Support
1. Research boss mechanics in AzerothCore source
2. Create boss database entry with validation rules
3. Add detection patterns to Python detector
4. Create test cases for validation
5. Update bash monitor patterns

### Improving Detection
1. Analyze missed detection cases
2. Refine regex patterns and rules
3. Add new violation types
4. Enhance evidence collection
5. Test with diverse strategy formats

## Version History

### v2.0 (Current)
- Boss Script Cross-Reference Module
- Python Strategic Pattern Detector
- Comprehensive mechanics database
- Twin Emperors error detection
- Integration suite

### v1.0 (Legacy)
- Basic CLAUDE.md compliance checking
- Simple violation detection
- Bash-only implementation

## License

This enhanced compliance monitor follows the same license as AzerothCore and is designed specifically for use with AzerothCore development workflows.