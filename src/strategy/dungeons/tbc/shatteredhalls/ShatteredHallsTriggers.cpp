#include "Playerbots.h"
#include "ShatteredHallsTriggers.h"
#include "ShatteredHallsActions.h"

bool NethekurseShadowFissureTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "grand warlock nethekurse");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    if (boss->HasAura(SPELL_SHADOW_FISSURE) || boss->FindCurrentSpellBySpellId(SPELL_SHADOW_FISSURE))
    {
        return true;
    }
    
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
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
    
    GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
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
    
    if (!boss->HasAura(SPELL_BURNING_MAUL))
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
    
    if (boss->HasAura(SPELL_BLADE_DANCE_DMG) || boss->FindCurrentSpellBySpellId(SPELL_BLADE_DANCE_DMG))
    {
        float distance = bot->GetExactDist2d(boss);
        return distance < 10.0f;
    }
    
    return false;
}

bool KargathAssassinsTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "warchief kargath bladefist");
    if (!boss || !boss->IsInCombat())
    {
        return false;
    }
    
    GuidVector attackers = AI_VALUE(GuidVector, "attackers");
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