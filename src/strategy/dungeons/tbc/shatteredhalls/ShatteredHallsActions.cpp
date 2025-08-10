#include "Playerbots.h"
#include "ShatteredHallsActions.h"
#include "ShatteredHallsStrategy.h"

bool AvoidShadowFissureAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "grand warlock nethekurse");
    if (!boss)
    {
        return false;
    }

    float safeDistance = 8.0f;
    
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
            float currentDist = bot->GetExactDist2d(unit);
            if (currentDist < safeDistance)
            {
                return MoveAway(unit, safeDistance - currentDist + 2.0f);
            }
        }
    }
    
    if (boss->HasAura(SPELL_SHADOW_FISSURE) || boss->FindCurrentSpellBySpellId(SPELL_SHADOW_FISSURE))
    {
        float currentDist = bot->GetExactDist2d(boss);
        if (currentDist < safeDistance)
        {
            return MoveAway(boss, safeDistance - currentDist + 2.0f);
        }
    }
    
    return false;
}

bool AvoidDarkSpinAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "grand warlock nethekurse");
    if (!boss)
    {
        return false;
    }
    
    if (!boss->HasAura(SPELL_DARK_SPIN) && !boss->FindCurrentSpellBySpellId(SPELL_DARK_SPIN))
    {
        return false;
    }
    
    float safeDistance = 10.0f;
    float currentDist = bot->GetExactDist2d(boss);
    
    if (currentDist < safeDistance)
    {
        return MoveAway(boss, safeDistance - currentDist + 2.0f);
    }
    
    return false;
}

bool MoveFromBlastWaveAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "warbringer o'mrogg");
    if (!boss)
    {
        return false;
    }
    
    if (!boss->HasAura(SPELL_BURNING_MAUL))
    {
        return false;
    }
    
    float safeDistance = 12.0f;
    float currentDist = bot->GetExactDist2d(boss);
    
    if (currentDist < safeDistance)
    {
        return MoveAway(boss, safeDistance - currentDist + 2.0f);
    }
    
    return false;
}

bool AvoidBurningMaulAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "warbringer o'mrogg");
    if (!boss)
    {
        return false;
    }
    
    if (boss->FindCurrentSpellBySpellId(SPELL_FEAR))
    {
        return false;
    }
    
    if (boss->HasAura(SPELL_BURNING_MAUL))
    {
        float safeDistance = 8.0f;
        float currentDist = bot->GetExactDist2d(boss);
        
        if (botAI->IsTank(bot))
        {
            if (currentDist > 5.0f)
            {
                return MoveTo(boss->GetMapId(), boss->GetPositionX(), boss->GetPositionY(), boss->GetPositionZ());
            }
        }
        else if (currentDist < safeDistance)
        {
            return MoveAway(boss, safeDistance - currentDist + 2.0f);
        }
    }
    
    return false;
}

bool AvoidBladeDanceAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "warchief kargath bladefist");
    if (!boss)
    {
        return false;
    }
    
    if (boss->HasAura(SPELL_BLADE_DANCE_DMG) || boss->FindCurrentSpellBySpellId(SPELL_BLADE_DANCE_DMG))
    {
        float safeDistance = 10.0f;
        float currentDist = bot->GetExactDist2d(boss);
        
        if (currentDist < safeDistance)
        {
            return MoveAway(boss, safeDistance - currentDist + 2.0f);
        }
    }
    
    return false;
}

bool KillShatteredAssassinsAction::isUseful()
{
    return !botAI->IsHeal(bot);
}

bool KillShatteredAssassinsAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "warchief kargath bladefist");
    if (!boss || !boss->IsInCombat())
    {
        return false;
    }
    
    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    
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
            if (currentTarget != unit)
            {
                return Attack(unit);
            }
            return false;
        }
    }
    
    return false;
}

bool NetheKursePeonPriorityAction::isUseful()
{
    return !botAI->IsHeal(bot);
}

bool NetheKursePeonPriorityAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "grand warlock nethekurse");
    if (!boss)
    {
        return false;
    }
    
    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    
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
            if (currentTarget != unit)
            {
                return Attack(unit);
            }
            return false;
        }
    }
    
    return false;
}