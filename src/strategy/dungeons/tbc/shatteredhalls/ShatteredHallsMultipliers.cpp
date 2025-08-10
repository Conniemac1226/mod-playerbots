#include "Playerbots.h"
#include "ShatteredHallsMultipliers.h"
#include "ShatteredHallsActions.h"
#include "SharedDefines.h"

float NethekurseMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "grand warlock nethekurse");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return 1.0f;
    }
    
    if (dynamic_cast<AvoidShadowFissureAction*>(action))
    {
        if (boss->HasAura(SPELL_SHADOW_FISSURE) || boss->FindCurrentSpellBySpellId(SPELL_SHADOW_FISSURE))
        {
            return 1.0f;
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
                    return 1.0f;
                }
            }
        }
        return 0.0f;
    }
    
    if (dynamic_cast<AvoidDarkSpinAction*>(action))
    {
        if (boss->GetHealthPct() <= 25.0f && (boss->HasAura(SPELL_DARK_SPIN) || boss->FindCurrentSpellBySpellId(SPELL_DARK_SPIN)))
        {
            return 1.0f;
        }
        return 0.0f;
    }
    
    if (dynamic_cast<NetheKursePeonPriorityAction*>(action))
    {
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
                return 1.0f;
            }
        }
        return 0.0f;
    }
    
    return 1.0f;
}

float OmroggMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "warbringer o'mrogg");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return 1.0f;
    }
    
    if (dynamic_cast<MoveFromBlastWaveAction*>(action))
    {
        if (boss->HasAura(SPELL_BURNING_MAUL))
        {
            float distance = bot->GetExactDist2d(boss);
            if (distance < 12.0f)
            {
                return 1.0f;
            }
        }
        return 0.0f;
    }
    
    if (dynamic_cast<AvoidBurningMaulAction*>(action))
    {
        if (boss->HasAura(SPELL_BURNING_MAUL) || boss->FindCurrentSpellBySpellId(SPELL_BURNING_MAUL))
        {
            return 1.0f;
        }
        return 0.0f;
    }
    
    return 1.0f;
}

float KargathMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "warchief kargath bladefist");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return 1.0f;
    }
    
    if (dynamic_cast<AvoidBladeDanceAction*>(action))
    {
        if (boss->HasAura(SPELL_BLADE_DANCE_DMG) || boss->FindCurrentSpellBySpellId(SPELL_BLADE_DANCE_DMG))
        {
            float distance = bot->GetExactDist2d(boss);
            if (distance < 10.0f)
            {
                return 1.0f;
            }
        }
        return 0.0f;
    }
    
    if (dynamic_cast<KillShatteredAssassinsAction*>(action))
    {
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
                return 1.0f;
            }
        }
        return 0.0f;
    }
    
    return 1.0f;
}