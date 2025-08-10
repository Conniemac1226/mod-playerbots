# DEVELOPMENT GUIDELINES - CRITICAL RULES

## CORE IMPLEMENTATION RULES

### 1. NO LAZY STUBS - 100% IMPLEMENTATION
- **NEVER** use lazy stub functions that only return `false`, `nullptr`, or `0` without proper logic
- **LEGITIMATE** validation returns are allowed: `if (!player) return false;`
- **NEVER** use comments like "// Simplified", "// TODO", "// Not implemented"
- **ALWAYS** implement complete functionality with proper conditional logic

### 2. AZEROTHCORE API RESEARCH - NO GUESSING
- **ALWAYS** research API usage from AzerothCore source before implementation
- **USE** `Acore::` namespace, NOT `Trinity::` for searchers
- **VERIFY** all spell IDs, NPC IDs, and constants exist in source
- **COPY** exact patterns from working AzerothCore examples
- **NEVER** fake implementations with hardcoded returns

Research Protocol:
```bash
grep -r "FunctionName" C:\Azerothcore\azerothcore-wotlk\src\
```

### 3. STRATEGIC ACCURACY VALIDATION
- **BOSS SCRIPT CROSS-REFERENCE**: Validate strategies against AzerothCore boss scripts
- **TANK ASSIGNMENT VERIFICATION**: Never assign tank roles to caster-only bosses  
- **POSITIONING COHERENCE**: Match positioning to boss mechanics (melee vs ranged)
- **MECHANICAL COMPLETENESS**: Include all critical encounter mechanics

### 4. BUILD RESPONSIBILITY
**USER HANDLES ALL BUILDS**: Never attempt to run build commands. Focus only on code implementation.

## STRATEGY IMPLEMENTATION PATTERNS

### VALIDATED MOVEMENT PATTERNS
```cpp
// Per-bot state maps - NEVER use global variables
std::map<ObjectGuid, uint32> g_boss_lastMoveTime;
std::map<ObjectGuid, bool> g_boss_inSafePosition;

// Simple hazard detection
for (Unit* unit : nearbyUnits) {
    if (unit->GetEntry() == HAZARD_CREATURE_ID) {
        return true;
    }
}

// Phase reset logic
if (g_boss_inSafePosition[botGuid] && hazardExists) {
    if ((currentTime - g_boss_lastMoveTime[botGuid]) > 10000) {
        g_boss_inSafePosition[botGuid] = false;
    }
}

// Movement patterns
FleePosition(position, distance, minTime);
MoveTo(mapId, x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
```

### PRIORITY SYSTEM LIMITS
```cpp
ACTION_EMERGENCY = 90  // Highest (90-94)
ACTION_INTERRUPT = 70  // Interrupts (70-74)
ACTION_DISPEL    = 65  // Dispels (65-69)
ACTION_HEAL      = 60  // Healing (60-64)
ACTION_MOVE      = 30  // Movement (30-39)
ACTION_NORMAL    = 20  // Normal (20-29)

// NEVER use multipliers > 1.0f
// NEVER exceed total priority of 95
```

### CRITICAL LESSONS LEARNED
- **State Management**: Per-bot state prevents multi-bot coordination issues
- **Priority Violations**: Never exceed 95 total priority (blocks chat commands at 100)
- **Simple Detection**: Direct entry ID checks work better than complex boss state logic
- **Phase Detection**: Time-based reset after 10+ seconds for repeating mechanics

### API PATTERNS FROM WOTLK
```cpp
// Boss targeting
Unit* boss = AI_VALUE2(Unit*, "find target", "boss name");

// Add detection
GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");

// Cell visiting (updated API)
Cell::VisitObjects(bot, searcher, range);  // NOT VisitAllObjects

// Aura checking
bot->HasAura(spellId)
boss->FindCurrentSpellBySpellId(spellId)
```

## KARAZHAN RAID EXAMPLE - ATTUMEN

Demonstrates proper research and implementation:

**RESEARCH FROM boss_midnight.cpp**:
- Phase 1: Midnight alone to 95%
- Phase 2: Both separate to 25%  
- Phase 3: Mounted form

**KEY MECHANICS**:
- Shadowcleave (29832): Frontal cleave
- Charge (29847): Targets 8-25 yard range
- Intangible Presence (29833): Fear

**IMPLEMENTATION STRUCTURE**:
```
src/strategy/raids/karazhan/
├── KarazhanActions.h/.cpp
├── KarazhanTriggers.h/.cpp  
├── KarazhanStrategy.h/.cpp
├── KarazhanMultipliers.h/.cpp
├── KarazhanActionContext.h
└── KarazhanTriggerContext.h
```

## COMPLIANCE VERIFICATION

### MANDATORY CHECKLIST
Before claiming completion:
- [ ] All functions have real conditional logic
- [ ] Zero stub functions with bare returns
- [ ] All APIs researched from AzerothCore source
- [ ] Boss mechanics cross-referenced with scripts
- [ ] Priority values within safe ranges (max 95)
- [ ] Per-bot state management implemented
- [ ] No global state corruption possible

### FORBIDDEN PATTERNS
```cpp
// ❌ LAZY STUB - BLOCKED
bool ProcessQuest(Player* player, uint32 questId) {
    return false;  // No logic, pure laziness
}

// ✅ LEGITIMATE VALIDATION - ALLOWED
bool ProcessQuest(Player* player, uint32 questId) {
    if (!player) {
        LOG_ERROR("module", "Invalid player");
        return false;  // Proper validation
    }
    // Implementation logic
    return ProcessQuestLogic(player, questId);
}
```

## TBC DUNGEON STRATEGIES - IN TESTING

### IMPLEMENTED TBC DUNGEONS
Following exact WOTLK patterns with researched boss mechanics:

**Shattered Halls**:
- Grand Warlock Nethekurse: Shadow Fissure avoidance, Lesser Shadow Fissure spread
- Warbringer O'mrogg: Burning Maul spread, Fear handling, Thunderclap mitigation
- Warchief Kargath: Blade Dance positioning, Reaver priority targeting

**Shadow Labyrinth**:
- Ambassador Hellmaw: Corrosive Acid spread, Fear management
- Blackheart the Inciter: Incite Chaos positioning, War Stomp avoidance
- Grandmaster Vorpil: Void Traveler adds, Draw Shadows/Rain of Fire movement, Banish handling
- Murmur: Sonic Boom positioning, Murmur's Touch spread, Resonance management

**Arcatraz**:
- Zereketh: Void Zone avoidance, Shadow Nova range check, Seed of Corruption spread
- Dalliah: Whirlwind avoidance, Heal interrupts, Gift of the Doomsayer handling
- Wrath-Scryer Soccothrates: Knock Away positioning, Felfire Charge spread
- Harbinger Skyriss: Illusion priority targeting, Fear/Domination management, Mind Rend positioning

**Botanica**:
- Commander Sarannis: Arcane Resonance handling, Reinforcement priority (Menders > Reservists)
- High Botanist Freywinn: Tree Form Frayer priority, Tranquility interrupt timing
- Laj: Allergic Reaction spread, Teleport positioning
- Thorngrin: Sacrifice handling, Hellfire avoidance, Enrage management
- Warp Splinter: War Stomp range check, Arcane Volley interrupts

### STATUS: IN TESTING
- All dungeons compile successfully
- API compatibility verified (MovementAction inheritance, InterruptSpell usage)
- Pattern compliance verified against WOTLK dungeons
- Priority values within safe limits (max 95)

## ENFORCEMENT

Work is **COMPLETE** when:
- ✅ All functions have proper conditional validation
- ✅ Zero lazy stub functions
- ✅ All APIs researched from source
- ✅ Strategic accuracy validated
- ✅ Priority system respected
- ✅ User can build and test successfully

**REMEMBER**: 
- Legitimate validation with conditions is REQUIRED
- Strategic accuracy is MANDATORY
- Always research before implementing