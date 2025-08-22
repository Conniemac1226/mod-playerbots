#include "MagistersTerraceTriggers.h"
#include "SpellInfo.h"
#include "Unit.h"
#include "Spell.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Playerbots.h"

// Kael'thas
bool KaelthasCastingPyroblastTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_KAELTHAS, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_PYROBLAST);
}

bool KaelthasCastingGravityLapseTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_KAELTHAS, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // CRITICAL: Primary detection is the flight aura (44227) - this means gravity lapse is active
    bool hasFlightAura = bot->HasAura(SPELL_GRAVITY_LAPSE_FLY);
    
    // Secondary detection: DOT aura (44226) - also indicates active gravity lapse
    bool hasDotAura = bot->HasAura(SPELL_GRAVITY_LAPSE_DOT);
    
    // Tertiary detection: Boss casting initial gravity lapse
    bool isCastingGravityLapse = boss->HasUnitState(UNIT_STATE_CASTING) && 
                                boss->FindCurrentSpellBySpellId(SPELL_GRAVITY_LAPSE);
    
    // Quaternary detection: Boss channeling gravity lapse (44251)
    bool isChannelingGravityLapse = boss->HasUnitState(UNIT_STATE_CASTING) && 
                                   boss->FindCurrentSpellBySpellId(44251); // SPELL_GRAVITY_LAPSE_CHANNEL
    
    
    // Return true if ANY gravity lapse effect is detected
    return hasFlightAura || hasDotAura || isCastingGravityLapse || isChannelingGravityLapse;
}

bool KaelthasArcaneSphereNearbyTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 15.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, 15.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_ARCANE_SPHERE)
            return true;
    }
    return false;
}

bool KaelthasMTFlamestrikeTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_KAELTHAS, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Check for flamestrike trigger creatures within dangerous range
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 12.0f); // 12 yard avoidance radius
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, 12.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_FLAMESTRIKE_TRIGGER)
            return true;
    }
    return false;
}

// Vexallus
bool VexallusPureEnergySpawnedTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if Vexallus is in combat first
    Unit* boss = bot->FindNearestCreature(NPC_VEXALLUS, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // ENHANCED DETECTION: Comprehensive Pure Energy detection
    bool pureEnergyFound = false;

    // Method 1: Check hostile NPCs list (most reliable)
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_PURE_ENERGY)
        {
            pureEnergyFound = true;
            break;
        }
    }

    // Method 2: Direct creature search with wider range
    if (!pureEnergyFound)
    {
        std::list<Unit*> targets;
        Acore::AnyUnitInObjectRangeCheck u_check(bot, 80.0f); // Increased range significantly
        Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
        Cell::VisitObjects(bot, searcher, 80.0f);

        for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
        {
            Unit* unit = *i;
            if (!unit || !unit->IsAlive())
                continue;

            // Accept ANY Pure Energy regardless of combat state
            if (unit->GetEntry() == NPC_PURE_ENERGY)
            {
                pureEnergyFound = true;
                break;
            }
        }
    }

    // CRITICAL FIX: DO NOT use aura detection - it causes false positives
    // Auras can persist after Pure Energy creatures die, blocking normal combat
    // Only rely on actual creature detection (Methods 1 & 2 above)
    
    // TIMEOUT MECHANISM: Track when Pure Energy was last seen to prevent stale triggers
    extern std::map<ObjectGuid, uint32> g_pureEnergy_lastSeenTime;
    ObjectGuid botGuid = bot->GetGUID();
    uint32 currentTime = getMSTime();
    
    if (pureEnergyFound)
    {
        // Update last seen time when Pure Energy is detected
        g_pureEnergy_lastSeenTime[botGuid] = currentTime;
    }
    else
    {
        // Check if we recently saw Pure Energy but haven't seen it for 3+ seconds
        if (g_pureEnergy_lastSeenTime[botGuid] > 0 && 
            (currentTime - g_pureEnergy_lastSeenTime[botGuid]) > 3000U)
        {
            // Clear the timestamp - Pure Energy phase is definitely over
            g_pureEnergy_lastSeenTime[botGuid] = 0;
        }
    }
    
    return pureEnergyFound;
}

// Selin Fireheart
bool SelinFireheartFelExplosionTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_SELIN_FIREHEART, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_FEL_EXPLOSION);
}

bool SelinFireheartChannelingTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = bot->FindNearestCreature(NPC_SELIN_FIREHEART, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Check if boss is channeling Mana Rage on a crystal
    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_MANA_RAGE);
}

bool FelCrystalNearbyTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if Selin is in combat and look for active crystals
    Unit* boss = bot->FindNearestCreature(NPC_SELIN_FIREHEART, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // ENHANCED DETECTION: Use multiple methods to find active crystals
    bool crystalFound = false;
    
    // Method 1: Direct creature search with Cell visiting
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 60.0f); // Increased range
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, 60.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        if (unit->GetEntry() == NPC_FEL_CRYSTAL)
        {
            // EXPANDED DETECTION: More comprehensive crystal state checking
            bool isSelectable = !unit->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE);
            bool isCasting = unit->HasUnitState(UNIT_STATE_CASTING) || unit->FindCurrentSpellBySpellId(SPELL_MANA_RAGE);
            bool isChanneling = unit->HasAura(SPELL_MANA_RAGE) || boss->HasAura(SPELL_MANA_RAGE);
            bool inCombat = unit->IsInCombat();
            bool hasTarget = unit->GetVictim() != nullptr;
            
            // Crystal is active if ANY of these conditions are met
            if (isSelectable || isCasting || isChanneling || inCombat || hasTarget)
            {
                crystalFound = true;
                break;
            }
        }
    }
    
    // Method 2: Check hostile NPCs list as backup
    if (!crystalFound)
    {
        const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
        for (auto& npc : npcs)
        {
            Unit* unit = botAI->GetUnit(npc);
            if (!unit || !unit->IsAlive())
                continue;
                
            if (unit->GetEntry() == NPC_FEL_CRYSTAL)
            {
                // Any fel crystal in hostile list is considered active
                crystalFound = true;
                break;
            }
        }
    }
    
    // Method 3: Boss state detection - if boss is channeling, there's likely an active crystal
    if (!crystalFound)
    {
        bool bossChanneling = boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_MANA_RAGE);
        bool bossHasAura = boss->HasAura(SPELL_MANA_RAGE);
        
        if (bossChanneling || bossHasAura)
        {
            crystalFound = true;
        }
    }

    return crystalFound;
}

// Delrissa
bool DelrissaAddActiveTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    // Check if Delrissa is engaged
    Unit* boss = bot->FindNearestCreature(NPC_DELRISSA, 100.0f);
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    // Check for her adds (various entries)
    const uint32 delrissaAdds[] = {24557, 24558, 24554, 24561, 24559, 24555, 24553, 24556};
    
    std::list<Unit*> targets;
    Acore::AnyUnitInObjectRangeCheck u_check(bot, 50.0f);
    Acore::UnitListSearcher<Acore::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitObjects(bot, searcher, 50.0f);

    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (!unit || !unit->IsAlive())
            continue;

        for (uint32 addId : delrissaAdds)
        {
            if (unit->GetEntry() == addId && unit->IsInCombat())
                return true;
        }
    }
    return false;
}