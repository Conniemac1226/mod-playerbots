# Gemini DEVELOPMENT GUIDELINES

## ABSOLUTE REQUIREMENTS - NO EXCEPTIONS

### 0. ANTI-LAZINESS RULE - NO WORKAROUNDS ALLOWED
- **NEVER** create new files to avoid fixing existing violations
- **NEVER** create "fixed" versions, "_temp" files, or workaround files  
- **ALWAYS** fix violations directly in the original file
- **NO EXCEPTIONS** - Fix the actual problem, don't work around it

### 1. 100% IMPLEMENTATION RULE
- **NEVER** use lazy stub functions that only return `false`, `nullptr`, or `0` without proper logic
- **LEGITIMATE** returns are allowed for validation: `if (!player) return false;`
- **NEVER** use comments like "// Simplified", "// TODO", "// Not implemented"
- **NEVER** claim "100% compatible" or "fully functional" unless EVERY feature is actually implemented
- **ALWAYS** implement complete functionality with proper conditional logic

### 2. HONESTY IN PROMISES
- **DO NOT** promise "100% features intact" unless you verify EVERY function works
- **DO NOT** say "fully compatible" when core systems are stubbed out
- **ALWAYS** audit your work before claiming completion
- **IMMEDIATELY** flag any unimplemented features to the user

### 3. IMPLEMENTATION STANDARDS
- **EVERY** function must have real logic with proper conditions, not bare placeholder returns
- **EVERY** system must integrate with the actual game engine APIs
- **EVERY** feature must be tested conceptually for correctness
- **ALL** complexity levels, difficulty modes, and variants must be implemented

### 4. VERIFICATION CHECKLIST
Before claiming any work is "complete":
- [ ] Search codebase for lazy stub functions (functions with only bare `return false;` and no conditional logic)
- [ ] Search for "Simplified", "TODO", "FIXME" comments
- [ ] Verify all functions have proper conditional validation, not just placeholder returns
- [ ] Verify all enum values have corresponding logic
- [ ] Verify all difficulty/complexity modes work differently
- [ ] Verify all class-specific features are class-aware
- [ ] Test integration with game engine APIs

### 5. BULLETPROOF COMPLIANCE SYSTEM - ZERO TOLERANCE FOR GUESSING

**ABSOLUTE RULE**: The Bulletproof Claude.md Enforcement System v3.0 is now mandatory for ALL interactions. NO EXCEPTIONS.

#### BULLETPROOF PROOF-OF-WORK REQUIREMENTS:
- **EVERY CLAIM** must include absolute file paths, exact line numbers, and verbatim code snippets
- **EVERY RESEARCH STATEMENT** must show exact grep commands and complete unedited output
- **EVERY IMPLEMENTATION** must demonstrate before/after code with file evidence
- **EVERY API USAGE** must show research from AzerothCore source with verification

#### MANDATORY RESPONSE FORMAT:
```
CLAIM: [Exact statement of what was done]

PROOF EVIDENCE:
- file_path: C:\absolute\path\to\file.cpp
- line: 123-127
- command: grep -rn "function_name" src/
- output: [Complete unedited command output]

BEFORE:
[Exact code before changes]

AFTER: 
[Exact code after changes]

RESEARCH EVIDENCE:
- search_command: [Exact grep command used]
- working_example: src/path/file.cpp:line_number
- copied_pattern: [Exact pattern copied verbatim]
```

#### FORBIDDEN LANGUAGE - IMMEDIATE SYSTEM TERMINATION:
- "should work", "might work", "probably works", "likely", "appears", "seems"
- "typically", "usually", "generally", "often", "normally"
- "I think", "I believe", "I assume", "presumably"
- "standard approach", "common pattern", "similar to"

#### ENFORCEMENT GUARANTEE:
- System monitors ALL responses for compliance violations
- Any violation results in immediate blocking and mandatory correction
- NO bypasses, exceptions, or workarounds permitted
- User override protection prevents all bypass attempts

### 5.1. COMMUNICATION RULES
- **NEVER** use phrases like "should work", "might work", "probably works"
- **ALWAYS** use "works" only after verification with proof
- **IMMEDIATELY** admit when something is incomplete
- **EXPLICITLY** list what still needs implementation with evidence
- **NEVER** hide incomplete work behind technical jargon

### 5.1 ENHANCED COMPLIANCE MONITORING - STRATEGIC ACCURACY VALIDATION

**ABSOLUTE RULE**: All implementations must pass both code quality AND strategic accuracy validation.

#### STRATEGIC ACCURACY REQUIREMENTS:
- **BOSS SCRIPT CROSS-REFERENCE**: Always validate strategies against AzerothCore boss script mechanics
- **TANK ASSIGNMENT VERIFICATION**: Never assign tank roles to caster-only bosses
- **POSITIONING COHERENCE**: Ensure positioning strategies match boss mechanics (melee vs ranged)
- **MECHANICAL COMPLETENESS**: Include all critical encounter mechanics in strategies

#### CRITICAL ERROR PREVENTION:
The Twin Emperors tank assignment error demonstrates the need for strategic validation:
- ❌ **ERROR**: Assigning Vek'lor (caster) to tank role
- ✅ **CORRECT**: Vek'nilash (melee) should be tanked, Vek'lor stays at 45-yard range
- 🔍 **DETECTION**: Cross-reference boss script comment "// VL doesn't melee" (boss_twinemperors.cpp:397)

#### MANDATORY STRATEGIC VALIDATION CHECKLIST:
Before claiming any strategy implementation is "complete":
- [ ] Cross-referenced boss script mechanics from AzerothCore source
- [ ] Verified tank assignments only go to melee-capable bosses
- [ ] Confirmed positioning requirements match boss range mechanics
- [ ] Validated all critical phases and mechanics are included
- [ ] Checked for contradictory positioning instructions
- [ ] Ensured role assignments match boss capabilities

#### BOSS MECHANICS RESEARCH PROTOCOL:
1. **LOCATE BOSS SCRIPT**: Find corresponding .cpp file in AzerothCore source
2. **EXTRACT MECHANICS**: Identify melee_capable, range requirements, special abilities
3. **VALIDATE STRATEGY**: Ensure strategy aligns with script mechanics
4. **DOCUMENT EVIDENCE**: Reference specific source lines for validation

#### STRATEGIC VIOLATION SEVERITY:
- **CRITICAL**: Tank assignments to caster-only bosses, role contradictions with boss scripts
- **HIGH**: Positioning contradictions, missing major mechanics
- **MEDIUM**: Missing optional mechanics, incomplete strategy details

### 6. AZEROTHCORE SPECIFIC RULES
- **ALWAYS** use proper AzerothCore APIs (Player::, Unit::, SpellMgr::)
- **ALWAYS** verify API usage by checking AzerothCore source code patterns
- **USE** `Acore::` namespace, NOT `Trinity::` for searchers and checkers
- **VERIFY** spell IDs, aura IDs, and constants exist in AzerothCore
- **CHECK** include patterns from existing AzerothCore scripts
- **NEVER** fake spell/aura checking with hardcoded returns
- **ALWAYS** implement proper rune/energy/mana/rage tracking using AC APIs
- **ALWAYS** implement real group/raid member detection using Group::GetFirstMember()
- **ALWAYS** implement actual totem/pet/minion detection using m_SummonSlot[]
- **NEVER** return fixed values for dynamic game state
- **ALWAYS** use SharedDefines.h constants (MAX_RUNES, SUMMON_SLOT_TOTEM_*, etc.)

### 6.1 MANDATORY AZEROTHCORE RESEARCH - NO GUESSING ALLOWED

**ABSOLUTE RULE**: NEVER guess how AzerothCore APIs work. ALWAYS research first.

#### BEFORE writing ANY AzerothCore code:
1. **RESEARCH the exact API usage**:
   ```bash
   grep -r "FunctionName" C:\Azerothcore\azerothcore-wotlk\src\
   ```
2. **FIND working examples** in existing scripts
3. **COPY the exact pattern** from verified working code
4. **VERIFY includes** from the working example
5. **CONFIRM constants** exist in SharedDefines.h or headers

#### MANDATORY VERIFICATION CHECKLIST:
- [ ] Found at least 1 working example of this API in AzerothCore source
- [ ] Copied exact namespace (Acore:: vs Trinity:: vs none)
- [ ] Copied exact include statements from working example
- [ ] Verified all spell IDs, aura IDs, constants exist in source
- [ ] Confirmed function signatures match exactly
- [ ] Tested the pattern works in similar context

#### RESEARCH SOURCES TO CHECK:
1. `/src/server/scripts/` - Working spell and creature scripts
2. `/src/server/game/` - Core game functionality 
3. `/src/server/shared/SharedDefines.h` - Constants and enums
4. `/src/server/game/Entities/Player/` - Player-specific APIs
5. `/src/server/game/Spells/` - Spell system APIs

#### WHEN RESEARCH FAILS:
If you cannot find a working example:
1. **IMMEDIATELY** tell the user "Cannot find AzerothCore example for X"
2. **DO NOT** implement based on guesswork
3. **ASK** user for guidance or alternative approach
4. **NEVER** fake it with stub returns

### 6.2 CLIENT-SIDE ADDON DEVELOPMENT - NO ASSUMPTIONS ALLOWED

**ABSOLUTE RULE**: Client-side WoW addon development follows the same NO GUESSING policy as server-side code.

#### BEFORE writing ANY WoW addon code for specific versions:
1. **RESEARCH the exact API compatibility** for the target WoW version
2. **FIND working examples** from addons that work on that version
3. **VERIFY function signatures** exist in that WoW version
4. **TEST basic functionality** before building complex features
5. **DOCUMENT what cannot be verified** without testing

#### WHEN CLIENT-SIDE RESEARCH FAILS:
1. **IMMEDIATELY** tell the user "Cannot verify [API] compatibility for [version]"
2. **DO NOT** implement based on assumptions about API compatibility
3. **ASK** user to test basic functionality first
4. **PROVIDE** incremental implementation starting with verified basics
5. **NEVER** claim "100% compatible" without testing

### 6.3 BUILD RESPONSIBILITY

**USER HANDLES ALL BUILDS**: The user will always handle building the project themselves. Claude should NEVER attempt to run build commands. Focus only on code implementation and fixes.

### 6.4 MANDATORY COMBAT IMPLEMENTATION REQUIREMENTS

**ABSOLUTE RULE**: ALL tank and DPS specializations MUST be implemented as CARBON COPIES of playerbots combat rotations. NO EXCEPTIONS.

### 6.5 VALIDATED TBC DUNGEON MOVEMENT SUCCESS - PRODUCTION READY

**COMPLETED IMPLEMENTATIONS**: Both Sethekk Halls and Auchenai Crypts movement strategies are now production-ready and deployed to GitHub.

#### SETHEKK HALLS - TALON KING IKISS ARCANE EXPLOSION:
✅ **USER-TESTED COORDINATES**: MoveTo with user-verified pillar coordinates that break line of sight  
✅ **PER-BOT STATE MANAGEMENT**: ObjectGuid-based maps prevent shared state corruption  
✅ **BUBBLE PHASE DETECTION**: Multi-phase reset logic handles repeated boss mechanics  
✅ **EMERGENCY FALLBACK**: Movement failure recovery with alternative positions
✅ **PRODUCTION READY**: Debug logging removed, clean GitHub-ready code

#### AUCHENAI CRYPTS - SHIRRAK FOCUS FIRE AVOIDANCE:
✅ **SIMPLE CREATURE DETECTION**: Direct NPC_FOCUS_FIRE (18374) existence check
✅ **ENVIRONMENTAL HAZARD APPROACH**: Treats Focus Fire as hazard, not combat target
✅ **PER-BOT STATE TRACKING**: Prevents repeated movement during same phase
✅ **MOVEMENT CALCULATION**: 15-yard radius from Focus Fire spawn location
✅ **ALTERNATIVE POSITIONING**: Fallback movement if primary position fails
✅ **PRODUCTION READY**: Debug logging removed, clean GitHub-ready code

#### CRITICAL SUCCESS FACTORS:
**KEY INSIGHT**: The core issue was **state management corruption**, not movement API problems. Multiple bots sharing global state variables caused only the first bot to move while others incorrectly assumed they were already safe.

#### WORKING PATTERNS VALIDATED:
1. **Per-bot state tracking**: `std::map<ObjectGuid, bool> g_*_inSafePosition` prevents state collision
2. **Phase transition detection**: Time-based reset logic detects new phases after 10+ seconds
3. **Dual reset mechanisms**: Both phase-end detection AND new-phase detection for reliability
4. **Simple creature detection**: Direct entry ID checks instead of complex boss state logic
5. **Clean production code**: No debug spam, professional comments, GitHub-ready

#### MANDATORY IMPLEMENTATION REQUIREMENTS:
For ANY multi-bot movement strategy:
- [x] **PER-BOT STATE**: Never use global variables for bot-specific state
- [x] **PHASE RESET LOGIC**: Detect when encounter phases repeat and reset state
- [x] **USER-TESTED COORDINATES**: Use coordinates verified to work by live testing
- [x] **EMERGENCY RECOVERY**: Handle movement failures with alternative positions
- [x] **SIMPLE DETECTION**: Use direct creature/aura checks, avoid complex boss state logic
- [x] **CLEAN CODE**: Remove debug logging before production deployment

#### VALIDATED MOVEMENT TEMPLATE:
```cpp
// Per-bot state maps - NEVER use global variables
std::map<ObjectGuid, uint32> g_boss_lastMoveTime;
std::map<ObjectGuid, bool> g_boss_inSafePosition;

// Simple hazard detection - no complex boss state checks
for (Unit* unit : nearbyUnits) {
    if (unit->GetEntry() == HAZARD_CREATURE_ID) {
        return true; // Simple, reliable detection
    }
}

// Per-bot phase reset logic
if (g_boss_inSafePosition[botGuid] && hazardExists) {
    if ((currentTime - g_boss_lastMoveTime[botGuid]) > 10000) {
        g_boss_inSafePosition[botGuid] = false; // New phase reset
    }
}

// User-tested safe positions
const Position SAFE_POSITIONS[] = {
    Position(x1, y1, z1),  // User-verified coordinates
    Position(x2, y2, z2),  // that actually work in-game
};
```

#### DEPLOYMENT STATUS:
- **Repository**: https://github.com/Conniemac1226/mod-playerbots
- **Branch**: enhanced-strategies  
- **Status**: Production ready, debug logging removed
- **Files**: 22 files changed, 817 insertions, 253 deletions
- **Commit**: "Clean up TBC dungeon strategies - remove debug logging"

#### CRITICAL PRIORITY VIOLATION LESSONS LEARNED (2025-08-07):
**NEVER AGAIN**: TBC Auchenai Crypts strategy caused severe command blocking and ghost server crashes due to:
- **Excessive Multipliers**: 100.0f multiplier (20x higher than working WotLK strategies)
- **Excessive Base Priority**: ACTION_MOVE + 10 (40 base priority vs 30-35 standard)
- **Infinite Action Loops**: `return true` when already safe caused continuous re-triggering
- **Effective Priority**: 4000 (40 × 100.0f) completely blocked 100 priority commands

**MANDATORY LIMITS FOR ALL FUTURE STRATEGIES**:
- **Multipliers**: NEVER exceed 10.0f (working WotLK max is 5.0f)
- **Base Priority**: Movement actions NEVER exceed ACTION_MOVE + 5 (35 total)
- **Action Returns**: `return false` when no action needed, `return true` only after successful execution
- **Effective Priority**: NEVER exceed 200 total priority (multiplier × base)
- **Command Priority**: Chat commands use ~100 priority - strategies must not monopolize queue

**COMMIT REFERENCE**: Fixed in commit f3e31ab6 - "fix: resolve TBC Auchenai Crypts priority violations"

#### SETHEKK HALLS GHOST FLEE MECHANIC - PRODUCTION READY ✅

**SETHEKK SPIRIT AVOIDANCE IMPLEMENTATION**:
- **Ghost Detection**: Sethekk Spirits (NPC ID 18703) spawn when Sethekk Prophets die
- **Flee Trigger**: SethekkSpiritNearbyTrigger detects spirits within 20 yards using Cell::VisitAllObjects
- **Flee Action**: FleeSpiritAction uses proven FleePosition() method from WotLK patterns
- **Safe Distance**: Flees 20 yards away from closest spirit with 500ms minimum interval
- **High Priority**: ACTION_EMERGENCY + 3 (priority 93) ensures immediate response

**IMPLEMENTATION PATTERN FOR GHOST/SPIRIT AVOIDANCE**:
```cpp
// Detection using Cell::VisitAllObjects (robust AzerothCore pattern)
std::list<Unit*> targets;
Acore::AnyUnitInObjectRangeCheck u_check(bot, 20.0f);
Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
Cell::VisitAllObjects(bot, searcher, 20.0f);

// Simple entry check for ghost/spirit entities
if (unit->GetEntry() == NPC_SETHEKK_SPIRIT) {
    return true; // Trigger flee action
}

// Use proven FleePosition method with appropriate distance
return FleePosition(closestSpirit->GetPosition(), 20.0f, 500U);
```

**VALIDATED SUCCESS FACTORS**:
- Uses identical patterns to working Forge of Souls and Pit of Saron flee mechanics
- Cell::VisitAllObjects matches proven totem detection in same dungeon
- FleePosition() is battle-tested in multiple WotLK dungeons
- Priority system prevents blocking while ensuring immediate response
- No per-bot state needed - FleePosition handles coordination internally

#### NPC HEALING STRATEGY REQUIREMENTS:
**KNOWN NPC HEALING ENCOUNTERS**:
- **Valithria Dreamwalker** (Icecrown Citadel) - FULLY IMPLEMENTED ✅
  - Reverse boss fight: heal dragon from 50% to 100% health
  - Complete healer bot rotations for all classes
  - Portal mechanics and dream state positioning
  - Production ready with multi-group coordination

**TBC NPC HEALING ENCOUNTERS**:
- **Thrall Rescue** (Escape from Durnholde Keep) - FIXED ✅
  - Direct CastSpell implementation replacing non-functional AI_VALUE2 calls
  - TBC Level 70 spell IDs for all healer classes
  - Health-based triggers at 60% and 25% thresholds
  - Status: User testing in progress

## KARAZHAN RAID IMPLEMENTATION - ATTUMEN THE HUNTSMAN ✅

**COMPLETED TBC RAID BOSS**: First boss of Karazhan fully implemented with proper AzerothCore research.

### ATTUMEN THE HUNTSMAN MECHANICS (RESEARCHED FROM boss_midnight.cpp)

**THREE-PHASE ENCOUNTER**:
1. **Phase 1 (Midnight alone)**: Fight horse until 95% health
2. **Phase 2 (Both separate)**: Attumen spawns, fight both until either reaches 25%
3. **Phase 3 (Mounted)**: They merge into single mounted boss

**KEY ABILITIES IMPLEMENTED**:
- **Shadowcleave (29832)**: Frontal cleave from Attumen - tanks position away, DPS stay behind
- **Intangible Presence (29833)**: Fear on random target - no specific counter needed
- **Knockdown (29711)**: From Midnight/Mounted - unavoidable ability
- **Charge (29847)**: Mounted phase only, targets players 8-25 yards away

### IMPLEMENTATION STRUCTURE

**FILES CREATED**:
```
src/strategy/raids/karazhan/
├── KarazhanActions.h/.cpp           # Boss-specific actions
├── KarazhanTriggers.h/.cpp          # Boss detection triggers  
├── KarazhanStrategy.h/.cpp          # Main strategy coordination
├── KarazhanMultipliers.h/.cpp       # Priority adjustments
├── KarazhanActionContext.h          # Action registration
└── KarazhanTriggerContext.h         # Trigger registration
```

**INTEGRATION POINTS**:
- Added to `RaidStrategyContext.h` as "karazhan" strategy
- Registered in `AiObjectContext.cpp` for action/trigger contexts
- Auto-loads in `PlayerbotAI.cpp` for map ID 532 (Karazhan)

### ATTUMEN STRATEGY IMPLEMENTATION

**CHARGE AVOIDANCE (Phase 3 Critical)**:
```cpp
// Mounted boss charges players between 8-25 yards
// Melee: Move to <8 yards (safe zone)
// Ranged: Move to >25 yards (safe zone)
bool AttumenAvoidChargeAction::Execute(Event event) {
    float distance = bot->GetDistance(boss);
    if (distance > 8.0f && distance < 25.0f) {
        if (botAI->IsMelee(bot)) {
            // Move closer to 5 yards
        } else {
            // Move further to 30 yards  
        }
    }
}
```

**POSITIONING STRATEGY (Phase 2)**:
```cpp
// Tanks: Separate bosses to avoid double cleave
// DPS: Stay behind Attumen to avoid Shadowcleave
bool AttumenPositionAction::Execute(Event event) {
    if (botAI->IsTank(bot)) {
        // Position bosses apart from each other
    } else {
        // Position behind Attumen for shadowcleave safety
    }
}
```

**TRIGGER SYSTEM**:
- `AttumenEngagedTrigger`: Detects phase 2 start (Attumen spawns)
- `AttumenMountedTrigger`: Detects phase 3 start (mounted form)
- `AttumenChargeDangerTrigger`: Detects charge danger zone
- `AttumenShadowcleaveTrigger`: Detects frontal cleave danger

**NPC IDS RESEARCHED FROM AZEROTHCORE**:
```cpp
enum KarazhanIDs {
    NPC_MIDNIGHT                = 16151,    // Phase 1 horse
    NPC_ATTUMEN_UNMOUNTED      = 15550,    // Phase 2 rider  
    NPC_ATTUMEN_MOUNTED        = 16152,    // Phase 3 combined
    
    SPELL_SHADOWCLEAVE         = 29832,    // Frontal cleave
    SPELL_INTANGIBLE_PRESENCE  = 29833,    // Fear effect
    SPELL_KNOCKDOWN            = 29711,    // Stun ability
    SPELL_CHARGE               = 29847,    // Mounted charge
};
```

**STRATEGY PRIORITIES**:
- Charge avoidance: ACTION_EMERGENCY (highest priority)
- General positioning: ACTION_MOVE + 2 (high priority)
- Shadowcleave avoidance: ACTION_MOVE + 1 (movement priority)

**STATUS**: Implementation completed with API fixes. Strategy auto-loads when entering Karazhan (map ID 532).

**BUILD FIXES APPLIED**:
- Fixed `Player::IsBehind()` compilation errors by using `Unit::HasInArc()` instead
- `HasInArc(M_PI / 2, bot)` checks if bot is in unit's front arc (for shadowcleave detection)
- All compilation errors resolved - ready for testing

**IMPLEMENTATION PATTERN FOR NPC HEALING**:
- Target friendly NPCs using `bot->FindNearestCreature(NPC_ID, 100.0f)`
- Use direct CastSpell instead of AI_VALUE2: `botAI->CastSpell(spellId, npc)`
- Implement proper spell IDs for each expansion (TBC Level 70 vs WotLK Level 80)
- Add NPC health monitoring triggers with `npc->GetHealthPct() < threshold`

#### REPLICATION REQUIREMENTS:
Use these implementations as templates for future dungeon movement strategies. The combination of per-bot state management, simple detection logic, and phase reset mechanisms solves multi-bot movement coordination problems.

#### REQUIRED SPECIALIZATIONS (27 TOTAL - ALL TANK/DPS, NO HEALERS):

**DEATH KNIGHT** (3 specs):
- Blood (Tank): Complete blood rune management, Death Strike healing, threat generation
- Frost (DPS): Obliterate/Frost Strike rotation, Killing Machine procs, dual-wield mechanics  
- Unholy (DPS): Disease management, Death Coil, Army of the Dead, pet coordination

**DRUID** (3 specs):
- Balance (DPS): Starfire/Wrath, Eclipse mechanics, Nature's Swiftness, mana management
- Feral (Tank): Bear form, Mangle/Swipe rotation, rage management, threat mechanics
- Feral (DPS): Cat form, combo points, Shred/Rake/Rip rotation, energy management

**HUNTER** (3 specs):
- Beast Mastery (DPS): Pet management, Bestial Wrath, Kill Command, focus management
- Marksmanship (DPS): Aimed Shot, Multi-Shot, Hunter's Mark, ranged mechanics
- Survival (DPS): Explosive Shot, traps, melee weaving, resource management

**MAGE** (3 specs):
- Arcane (DPS): Arcane Blast stacking, mana management, Arcane Power cooldowns
- Fire (DPS): Fireball/Fire Blast, Hot Streak procs, combustion mechanics
- Frost (DPS): Frostbolt/Ice Lance, Water Elemental, freeze mechanics

**PALADIN** (2 specs):
- Protection (Tank): Consecration, Shield of Righteousness, mana/threat balance
- Retribution (DPS): Seal twisting, Consecration, two-handed weapon mechanics

**PRIEST** (2 specs):
- Shadow (DPS): Shadow Word: Pain/Death, Mind Blast, shadow orb management
- Discipline (DPS): Smite/Holy Fire, spiritual guidance, mana efficiency

**ROGUE** (3 specs):
- Assassination (DPS): Poison management, Mutilate, energy/combo optimization
- Combat (DPS): Sinister Strike, Blade Flurry, weapon expertise mechanics
- Subtlety (DPS): Stealth mechanics, Shadow Dance, positional requirements

**SHAMAN** (2 specs):
- Enhancement (DPS): Dual-wield, Stormstrike, totem management, mana/melee hybrid
- Elemental (DPS): Lightning Bolt/Chain Lightning, Lava Lash, totem optimization

**WARLOCK** (3 specs):
- Affliction (DPS): DoT management, Soul Burn, life tap mechanics, pet coordination
- Demonology (DPS): Metamorphosis, demon form, pet empowerment, soul link
- Destruction (DPS): Shadow Bolt/Incinerate, chaos bolt, soul shard management

**WARRIOR** (3 specs):
- Protection (Tank): Shield Slam, Devastate, rage management, threat generation
- Arms (DPS): Mortal Strike, Execute, two-handed weapon mastery, stance management
- Fury (DPS): Dual-wield, Bloodthirst, Whirlwind, berserker stance mechanics

#### CARBON COPY REQUIREMENTS:
Each specialization MUST implement:
- **SPELL-BY-SPELL EXACTNESS**: Every spell in playerbots rotation with identical priority order
- **THRESHOLD MATCHING**: Health %, mana %, energy values identical to playerbots source
- **TIMING PRECISION**: Cooldown usage, GCD handling, cast time calculations exact
- **CONDITIONAL LOGIC**: If/then statements copied verbatim from playerbots
- **BUFF/DEBUFF CHECKING**: Aura detection logic identical to playerbots patterns
- **RESOURCE CALCULATIONS**: Mana/rage/energy math formulas copied exactly
- **AOE CAPABILITIES**: Multi-target detection and rotation switching exactly as playerbots
- **THREAT AWARENESS**: Tank threat generation, DPS threat management exactly as playerbots
- **PET/TOTEM AI**: Hunter pets, warlock demons, shaman totems exactly as playerbots
- **STANCE/FORM MANAGEMENT**: Warrior stances, druid forms, paladin seals exactly as playerbots

#### MANDATORY RESEARCH PROTOCOL:
Before implementing ANY combat class:
1. **EXTRACT PLAYERBOTS SOURCE**: Copy exact spell lists, priorities, and conditions from mod-playerbots
2. **VERIFY SPELL IDS**: Confirm all spell IDs exist in AzerothCore database
3. **COPY RESOURCE CHECKS**: Use identical mana/rage/energy thresholds
4. **MATCH CONDITIONAL LOGIC**: Copy exact if/then conditions for spell usage
5. **IMPLEMENT BUFF DETECTION**: Use identical aura checking patterns

#### COMPLIANCE VERIFICATION REQUIREMENTS:
The compliance agent MUST verify:
- [ ] All 27 specializations extracted from playerbots source code
- [ ] Side-by-side comparison shows 100% spell order matching
- [ ] All numerical thresholds (health %, mana %, etc.) identical to playerbots
- [ ] Conditional logic copied verbatim from playerbots source
- [ ] No approximations or "similar" implementations accepted
- [ ] Source code references provided for each rotation
- [ ] All spell IDs verified in AzerothCore database
- [ ] Resource management thresholds match playerbots exactly
- [ ] Buff/debuff management identical to playerbots patterns
- [ ] AOE detection and multi-target logic implemented exactly as playerbots
- [ ] Pet/totem AI matches playerbots behavior exactly
- [ ] Cooldown usage patterns identical to playerbots timing

#### VIOLATION DETECTION:
Any implementation claiming "playerbots-compatible" or "carbon copy" without:
- Complete rotations for all abilities
- Proper resource management
- Buff/debuff systems
- AOE capabilities
- Pet/totem coordination
Will be flagged as a CRITICAL CLAUDE.md violation requiring immediate correction.

### 7. WHEN THINGS ARE INCOMPLETE
If you cannot implement something fully:
1. **IMMEDIATELY** tell the user what's missing
2. **EXPLICITLY** list all non-functional stubs
3. **PROVIDE** estimated effort to complete
4. **ASK** for permission to implement incrementally
5. **NEVER** hide incomplete work

### 8. QUALITY GATES
No code is "done" until:
- All functions have real implementations with proper conditional logic
- All game states are properly detected
- All class/spec variations work correctly
- All complexity/difficulty modes are different
- All error cases are handled
- All edge cases are considered
- **ALL STRATEGIC ACCURACY REQUIREMENTS VERIFIED** (Section 5.1)

### 9. ENHANCED COMPLIANCE MONITORING INTEGRATION

The Enhanced Claude MD Compliance Suite (v2.0) provides automated detection of both code quality violations and strategic accuracy errors.

#### COMPONENTS INCLUDED:
- **enhanced-compliance-suite.sh**: Main integration script for all monitoring
- **claude-md-compliance-monitor-enhanced.sh**: Enhanced bash monitor with boss script cross-reference
- **strategic-pattern-detector.py**: Python detector for strategic violations
- **boss-mechanics-database.json**: Comprehensive boss mechanics database

#### AUTOMATIC DETECTION CAPABILITIES:
- **Tank Assignment Errors**: Catches Vek'lor tank assignment and similar violations
- **Positioning Contradictions**: Detects melee assignments for ranged bosses
- **Missing Critical Mechanics**: Flags strategies missing encounter-defining features
- **Boss Script Cross-Reference**: Validates strategies against AzerothCore source

#### USAGE REQUIREMENTS:
Run enhanced compliance checking after ANY strategy implementation:
```bash
./enhanced-compliance-suite.sh check [path]
./enhanced-compliance-suite.sh test-twin-emperors
```

The compliance monitor MUST PASS before claiming any strategy work is complete.

## LEGITIMATE VS LAZY CODE EXAMPLES

### ✅ LEGITIMATE VALIDATION CODE (ALLOWED):
```cpp
bool ProcessQuest(Player* player, uint32 questId) {
    if (!player) {
        LOG_ERROR("module", "ProcessQuest: Invalid player");
        return false;  // ✅ GOOD - Proper validation with logging
    }
    
    if (questId == 0) {
        LOG_WARN("module", "ProcessQuest: Invalid quest ID");
        return false;  // ✅ GOOD - Parameter validation
    }
    
    if (!player->IsInWorld()) {
        return false;  // ✅ GOOD - Game state validation
    }
    
    // Actual implementation logic here
    return ProcessQuestLogic(player, questId);
}
```

### ❌ LAZY STUB CODE (BLOCKED):
```cpp
bool ProcessQuest(Player* player, uint32 questId) {
    return false;  // ❌ BAD - No logic, no validation, pure laziness
}

bool AdvancedFeature() {
    // TODO: implement later
    return false;  // ❌ BAD - Placeholder comment + lazy return
}

bool ComplexSystem() {
    // Simplified version
    return false;  // ❌ BAD - Admitting incomplete implementation
}
```

## ENFORCEMENT SUCCESS CRITERIA

Work is considered **COMPLETE** when:
- ✅ All functions have proper conditional validation logic
- ✅ Zero lazy stub functions (functions with only bare returns)
- ✅ Zero TODO/FIXME/Simplified comments
- ✅ All APIs researched from AzerothCore source
- ✅ All claims verified and honest
- ✅ **STRATEGIC ACCURACY VALIDATED** - Enhanced compliance monitoring passed
- ✅ **BOSS SCRIPT CROSS-REFERENCE COMPLETE** - Strategy aligns with AzerothCore mechanics
- ✅ **TANK ASSIGNMENTS VERIFIED** - No caster bosses assigned tank roles
- ✅ User can build and test successfully

**REMEMBER**: 
- Legitimate validation returns with proper conditions are REQUIRED for good code. Only lazy stub functions without logic are forbidden.
- **STRATEGIC ACCURACY IS MANDATORY** - The Twin Emperors tank assignment error must never occur again.
- **USE THE ENHANCED COMPLIANCE SUITE** - Run `./enhanced-compliance-suite.sh check` before claiming completion.

## RECENT PROJECT WORK COMPLETED (2025-08-08)

### 🔧 **PLAYERBOTS MODULE FIXES - PRODUCTION READY**

#### **1. Reverted Problematic Performance Commits**
- ✅ **COMPLETED**: Reverted commits a2f7b8c1 and f5ef5bd1 causing server latency spikes
- ✅ **VERIFIED**: Bot AI cleanup and access violation fixes were causing excessive CPU usage
- ✅ **STATUS**: Performance issues resolved, server stability restored

#### **2. TBC Dungeon Strategies - Priority Violations Fixed**
- ✅ **ESCAPE FROM DURNHOLDE**: Complete strategy overhaul following proven patterns
  - Added per-bot state management (std::map<ObjectGuid, bool>) to prevent bot stacking
  - Fixed movement API to match Sethekk Halls/Auchenai Crypts patterns
  - Added return position actions and triggers
  - All priorities within safe limits (max: ACTION_EMERGENCY + 3 = 93)

#### **3. Thrall NPC Healing - FIXED** ✅
- ✅ **IMPLEMENTATION**: Direct `botAI->CastSpell(spellId, npc)` replacing non-functional AI_VALUE2 calls
- ✅ **SPELL IDS**: TBC Level 70 spells for all healer classes (Priest: Renew 25315, Greater Heal 25314)
- ✅ **TRIGGERS**: Health-based healing at 60% and 25% thresholds
- ⏳ **STATUS**: User testing Thrall healing on test server

- ✅ **SETHEKK HALLS**: Fixed dangerous multiplier stacking causing 420+ effective priority
  - Removed 10.0f totem multiplier and 5.0f Ikiss movement multiplier
  - Changed base priorities: Totem attack now ACTION_EMERGENCY + 2 (92 priority)
  - Follows WotLK dungeon patterns (no multipliers for target switching)
  - All effective priorities now safe (max: 93, won't block 100-priority commands)

#### **4. Sethekk Halls Ranged Totem Targeting - IN TESTING** 🧪
- ⏳ **TESTING PHASE**: Added movement logic for ranged classes to attack Charming Totems
- **IMPLEMENTATION**: 
  - MoveToCharmingTotemAction for positioning ranged bots within 20-25 yard range
  - Enhanced AttackCharmingTotemAction with range checking
  - Uses proven MoveTo() API patterns from other working strategies
- **TESTING NOTES**: 
  - MoveTo() API usage matches proven Sethekk/Auchenai/EFD patterns ✅
  - Range logic simplified but follows WotLK patterns ✅
  - May need adjustment based on test server results
  - Monitoring for potential action conflicts or movement loops
- **FALLBACK**: Can remove movement additions if testing shows issues

#### **5. Priority Safety Analysis - VERIFIED SAFE**
```
BEFORE (Dangerous):
- Sethekk Totem: 42 × 10.0 = 420 priority (BLOCKED COMMANDS) ❌
- Ikiss Movement: 36 × 5.0 = 180 priority ❌

AFTER (Safe):
- Sethekk Totem: 92 × 1.0 = 92 priority ✅
- EFD Movement: 93 × 1.0 = 93 priority ✅  
- Chat Commands: ~100 priority (NOT BLOCKED) ✅
```

#### **6. Code Quality Standards Met**
- ✅ All implementations follow proven working strategy patterns
- ✅ Per-bot state management prevents multi-bot coordination issues
- ✅ No multiplier stacking or priority violations
- ✅ API usage researched from working AzerothCore examples
- ✅ No lazy stub functions or incomplete implementations

### 📊 **DEPLOYMENT STATUS**
- **Repository**: https://github.com/Conniemac1226/mod-playerbots  
- **Branch**: enhanced-strategies
- **Commits**: 
  - `a307e40e` - Priority violation fixes (PRODUCTION READY)
  - Totem movement enhancements (PENDING TEST SERVER VALIDATION)
- **Status**: Core fixes deployed, ranged movement testing in progress