#include "Playerbots.h"
#include "ArcatrazTriggers.h"
#include "ArcatrazActions.h"

bool ZerekethVoidZoneTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "zereketh the unbound");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit)
        {
            continue;
        }
        
        if (unit->GetName() == "Void Zone")
        {
            float distance = bot->GetExactDist2d(unit);
            if (distance < 10.0f)
            {
                return true;
            }
        }
    }
    
    return false;
}

bool ZerekethShadowNovaTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "zereketh the unbound");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    if (boss->FindCurrentSpellBySpellId(ARC_SPELL_SHADOW_NOVA))
    {
        float distance = bot->GetExactDist2d(boss);
        return distance < 20.0f;
    }
    
    return false;
}

bool ZerekethSeedOfCorruptionTrigger::IsActive()
{
    if (!bot->HasAura(SPELL_SEED_OF_CORRUPTION))
    {
        return false;
    }
    
    GuidVector members = AI_VALUE(GuidVector, "group members");
    for (auto& member : members)
    {
        if (member == bot->GetGUID())
        {
            continue;
        }
        
        Unit* unit = botAI->GetUnit(member);
        if (!unit)
        {
            continue;
        }
        
        float distance = bot->GetExactDist2d(unit);
        if (distance < 10.0f)
        {
            return true;
        }
    }
    
    return false;
}

bool DalliahWhirlwindTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "dalliah the doomsayer");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    // RESEARCHED: boss_dalliah_the_doomsayer.cpp:101 - DoCastAOE(ARC_SPELL_WHIRLWIND) - instant AoE cast!
    // Whirlwind is an INSTANT AoE ability, not channeled - check for aura instead of casting state
    
    float distance = bot->GetExactDist2d(boss);
    
    // Check if boss has whirlwind aura (the spinning effect)
    if (boss->HasAura(ARC_SPELL_WHIRLWIND))
    {
        return distance < 15.0f; // Increased safety range - whirlwind hits everything in melee range
    }
    
    // Also check if boss just said the whirlwind emote (backup detection)
    // This provides earlier warning since emote happens before the actual cast
    if (boss->HasUnitState(UNIT_STATE_CASTING))
    {
        return distance < 12.0f; // Move early if casting anything while close
    }
    
    return false;
}

bool DalliahHealTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "dalliah the doomsayer");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    return boss->FindCurrentSpellBySpellId(ARC_SPELL_HEAL);
}

bool SoccothratesKnockAwayTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "wrath-scryer soccothrates");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    if (boss->FindCurrentSpellBySpellId(ARC_SPELL_KNOCK_AWAY))
    {
        float distance = bot->GetExactDist2d(boss);
        return distance < 15.0f;
    }
    
    return false;
}

bool SoccothratesChargeTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "wrath-scryer soccothrates");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    if (boss->HasAura(SPELL_FELFIRE) || boss->FindCurrentSpellBySpellId(ARC_SPELL_CHARGE))
    {
        GuidVector members = AI_VALUE(GuidVector, "group members");
        for (auto& member : members)
        {
            if (member == bot->GetGUID())
            {
                continue;
            }
            
            Unit* unit = botAI->GetUnit(member);
            if (!unit)
            {
                continue;
            }
            
            float distance = bot->GetExactDist2d(unit);
            if (distance < 8.0f)
            {
                return true;
            }
        }
    }
    
    return false;
}

bool FelfireGroundTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "wrath-scryer soccothrates");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    // RESEARCHED: NPC ID 20978 "Wrath-Scryer's Felfire" - persistent ground fire effects
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit)
        {
            continue;
        }
        
        if (unit->GetEntry() == NPC_FELFIRE_GROUND && unit->IsAlive())
        {
            float distance = bot->GetExactDist2d(unit);
            if (distance < 8.0f) // Stay away from persistent fire zones
            {
                return true;
            }
        }
    }
    
    return false;
}

bool SkyrissIllusionTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "harbinger skyriss");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit)
        {
            continue;
        }
        
        if (unit->GetEntry() == NPC_HARBINGER_ILLUSION && unit->IsAlive())
        {
            return true;
        }
    }
    
    return false;
}

bool SkyrissFearTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "harbinger skyriss");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    return boss->FindCurrentSpellBySpellId(ARC_SPELL_FEAR);
}

bool SkyrissDominationTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "harbinger skyriss");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    return boss->FindCurrentSpellBySpellId(ARC_SPELL_DOMINATION) || bot->HasAura(ARC_SPELL_DOMINATION);
}

bool MellicharAddsActiveTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
        
    // Check if we're fighting Warden Mellichar OR Harbinger Skyriss (final encounter)
    Unit* warden = AI_VALUE2(Unit*, "find target", "warden mellichar");
    Unit* skyriss = AI_VALUE2(Unit*, "find target", "harbinger skyriss");
    
    if ((!warden || !warden->IsInCombat()) && (!skyriss || !skyriss->IsInCombat()))
        return false;
        
    // WOTLK PATTERN: Use "possible targets" for better add detection
    const GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    const uint32 mellicharAdds[] = {
        NPC_TRICKSTER, NPC_PH_HUNTER,      // Phase 1 
        NPC_AKKIRIS, NPC_SULFURON,         // Phase 3
        NPC_TW_DRAK, NPC_BL_DRAK,          // Phase 4
        NPC_HARBINGER_SKYRISS               // Phase 5 - Final boss
        // NOTE: NPC_MILLHOUSE excluded - becomes friendly!
    };
    
    for (auto& targetGuid : targets)
    {
        Unit* unit = botAI->GetUnit(targetGuid);
        if (!unit || !unit->IsAlive())
            continue;

        for (uint32 addId : mellicharAdds)
        {
            if (unit->GetEntry() == addId)
                return true;
        }
    }
    
    return false;
}

bool MellicharImmuneTrigger::IsActive()
{
    Unit* warden = AI_VALUE2(Unit*, "find target", "warden mellichar");
    if (!warden || !warden->IsAlive() || !warden->IsInCombat())
        return false;

    // Warden Mellichar is immune when he has the "Banished" aura (spell id 36645)
    return warden->HasAura(36645);
}