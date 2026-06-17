#include "Playerbots.h"
#include "ShatteredHallsTriggers.h"
#include "ShatteredHallsActions.h"

// Constants are already defined in ShatteredHallsActions.h

bool NethekurseShadowFissureTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "grand warlock nethekurse");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    if (boss->HasAura(SH_SPELL_SHADOW_FISSURE) || boss->FindCurrentSpellBySpellId(SH_SPELL_SHADOW_FISSURE))
    {
        return true;
    }
    
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit)
        {
            continue;
        }
        
        if (unit->HasAura(SPELL_LESSER_SHADOW_FISSURE))
        {
            float distance = bot->GetExactDist2d(unit);
            if (distance < 8.0f)
            {
                return true;
            }
        }
    }
    
    return false;
}

bool NethekurseDarkSpinTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "grand warlock nethekurse");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    if (boss->GetHealthPct() > 25.0f)
    {
        return false;
    }
    
    return boss->HasAura(SPELL_DARK_SPIN) || boss->FindCurrentSpellBySpellId(SPELL_DARK_SPIN);
}

bool NethekursePeonsTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "grand warlock nethekurse");
    if (!boss)
    {
        return false;
    }
    
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (!unit)
        {
            continue;
        }
        
        if (unit->GetEntry() == NPC_PEON && unit->IsAlive() && unit->IsInCombat())
        {
            return true;
        }
    }
    
    return false;
}

bool OmroggBlastWaveTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "warbringer o'mrogg");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    if ((!boss->HasAura(SPELL_BURNING_MAUL) &&
         !boss->FindCurrentSpellBySpellId(SPELL_BURNING_MAUL)) ||
        !boss->HasUnitState(UNIT_STATE_CASTING) ||
        !boss->FindCurrentSpellBySpellId(SPELL_BLAST_WAVE))
    {
        return false;
    }
    
    float distance = bot->GetExactDist2d(boss);
    return distance < 12.0f;
}

bool OmroggBurningMaulTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "warbringer o'mrogg");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    return boss->HasAura(SPELL_BURNING_MAUL) || boss->FindCurrentSpellBySpellId(SPELL_BURNING_MAUL);
}

bool KargathBladeDanceTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "warchief kargath bladefist");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    if (boss->HasUnitState(UNIT_STATE_CASTING) &&
        boss->FindCurrentSpellBySpellId(SPELL_BLADE_DANCE_TARGETING))
    {
        float distance = bot->GetExactDist2d(boss);
        return distance < 10.0f;
    }
    
    return false;
}

bool KargathAssassinsTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "warchief kargath bladefist");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    const GuidVector attackers = AI_VALUE(GuidVector, "attackers");
    for (auto& attacker : attackers)
    {
        Unit* unit = botAI->GetUnit(attacker);
        if (!unit)
        {
            continue;
        }
        
        if (unit->GetEntry() == NPC_SHATTERED_ASSASSIN)
        {
            return true;
        }
    }
    
    return false;
}

bool FlameArrowGauntletTrigger::IsActive()
{
    if (bot->HasAura(SPELL_FLAME_ARROW_FIRE))
    {
        return true;
    }

    // Check if we are in the gauntlet area with active archers
    const GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    for (auto& guid : npcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;
            
        // Check for active archers shooting flame arrows
        if (unit->GetEntry() == NPC_SH_ARCHER)
        {
            if (unit->FindCurrentSpellBySpellId(SPELL_SHOOT_FLAME_ARROW))
                return true;
        }
    }
    
    // Also check if there are fire patches on the ground nearby
    const GuidVector allNpcs = AI_VALUE(GuidVector, "nearest npcs");
    for (auto& guid : allNpcs)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit)
            continue;
            
        // Check for fire effect units/triggers
        if (unit->HasAura(SPELL_FLAME_ARROW_FIRE))
        {
            if (bot->GetDistance(unit) < 15.0f)
                return true;
        }

        // Common fire trigger NPCs
        if (unit->GetEntry() == NPC_SH_FLAME_ARROW)
        {
            if (bot->GetDistance(unit) < 15.0f)
                return true;
        }
    }

    // Check for fire game objects
    std::list<GameObject*> gameObjects;
    bot->GetGameObjectListWithEntryInGrid(gameObjects, GO_BLAZE, 15.0f);
    if (!gameObjects.empty())
        return true;

    return false;
}
