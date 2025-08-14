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
    
    if (boss->FindCurrentSpellBySpellId(SPELL_SHADOW_NOVA))
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
    
    // RESEARCHED: boss_dalliah_the_doomsayer.cpp:100-101 - DoCastAOE(SPELL_WHIRLWIND)
    // Expanded trigger range to match successful Drake pattern (15 yard detection)
    
    // Priority 1: Detect casting for maximum reaction time
    if (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_WHIRLWIND))
    {
        float distance = bot->GetExactDist2d(boss);
        return distance < 15.0f; // Increased from 10 to 15 yards for earlier detection
    }
    
    // Priority 2: Active whirlwind (emergency escape)
    if (boss->HasAura(SPELL_WHIRLWIND))
    {
        float distance = bot->GetExactDist2d(boss);
        return distance < 12.0f; // Increased from 8 to 12 yards
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
    
    return boss->FindCurrentSpellBySpellId(SPELL_HEAL);
}

bool SoccothratesKnockAwayTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "wrath-scryer soccothrates");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    if (boss->FindCurrentSpellBySpellId(SPELL_KNOCK_AWAY))
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
    
    if (boss->HasAura(SPELL_FELFIRE) || boss->FindCurrentSpellBySpellId(SPELL_CHARGE))
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
    
    return boss->FindCurrentSpellBySpellId(SPELL_FEAR);
}

bool SkyrissDominationTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "harbinger skyriss");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    return boss->FindCurrentSpellBySpellId(SPELL_DOMINATION) || bot->HasAura(SPELL_DOMINATION);
}

bool MellicharAddsActiveTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
        
    // Check if we're fighting Warden Mellichar (who is immune)
    Unit* warden = AI_VALUE2(Unit*, "find target", "warden mellichar");
    if (!warden || !warden->IsInCombat())
        return false;
        
    // Check for any of Mellichar's adds
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    const uint32 mellicharAdds[] = {
        NPC_TRICKSTER, NPC_PH_HUNTER, 
        NPC_AKKIRIS, NPC_SULFURON, NPC_TW_DRAK, NPC_BL_DRAK
    };
    
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
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