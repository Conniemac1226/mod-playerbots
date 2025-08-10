#include "Playerbots.h"
#include "ShadowLabyrinthMultipliers.h"
#include "ShadowLabyrinthActions.h"
#include "SharedDefines.h"

float HellmawMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "ambassador hellmaw");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return 1.0f;
    }
    
    if (dynamic_cast<AvoidCorrosiveAcidAction*>(action))
    {
        if (boss->FindCurrentSpellBySpellId(SPELL_CORROSIVE_ACID) && boss->GetVictim() == bot)
        {
            return 1.0f;
        }
        return 0.0f;
    }
    
    if (dynamic_cast<HellmawFearReactAction*>(action))
    {
        if (boss->FindCurrentSpellBySpellId(SPELL_FEAR))
        {
            return 1.0f;
        }
        return 0.0f;
    }
    
    return 1.0f;
}

float BlackheartMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "blackheart the inciter");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return 1.0f;
    }
    
    if (dynamic_cast<InciteChaosReactAction*>(action))
    {
        if (boss->FindCurrentSpellBySpellId(SPELL_INCITE_CHAOS) || bot->HasAura(SPELL_INCITE_CHAOS_B))
        {
            return 1.0f;
        }
        return 0.0f;
    }
    
    if (dynamic_cast<AvoidWarStompAction*>(action))
    {
        if (boss->FindCurrentSpellBySpellId(SPELL_WAR_STOMP))
        {
            float distance = bot->GetExactDist2d(boss);
            if (distance < 10.0f)
            {
                return 1.0f;
            }
        }
        return 0.0f;
    }
    
    return 1.0f;
}

float VorpilMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "grandmaster vorpil");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return 1.0f;
    }
    
    if (dynamic_cast<VoidTravelerPriorityAction*>(action))
    {
        GuidVector npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
        for (auto& npc : npcs)
        {
            Unit* unit = botAI->GetUnit(npc);
            if (!unit)
            {
                continue;
            }
            
            if (unit->GetEntry() == NPC_VOID_TRAVELER)
            {
                return 1.0f;
            }
        }
        return 0.0f;
    }
    
    if (dynamic_cast<MoveFromRainOfFireAction*>(action))
    {
        if (boss->HasAura(SPELL_RAIN_OF_FIRE) || boss->FindCurrentSpellBySpellId(SPELL_RAIN_OF_FIRE))
        {
            Position centerPos = {-253.548f, -263.646f, 17.0864f};
            float distance = bot->GetExactDist2d(centerPos.GetPositionX(), centerPos.GetPositionY());
            if (distance < 12.0f)
            {
                return 1.0f;
            }
        }
        return 0.0f;
    }
    
    if (dynamic_cast<DrawShadowsReactAction*>(action))
    {
        if (boss->HasAura(SPELL_DRAW_SHADOWS) || boss->FindCurrentSpellBySpellId(SPELL_DRAW_SHADOWS))
        {
            return 1.0f;
        }
        return 0.0f;
    }
    
    return 1.0f;
}

float MurmurMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "murmur");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return 1.0f;
    }
    
    if (dynamic_cast<MurmurSonicBoomAction*>(action))
    {
        if (boss->HasAura(SPELL_SONIC_BOOM_CAST) || boss->FindCurrentSpellBySpellId(SPELL_SONIC_BOOM_CAST))
        {
            float distance = bot->GetExactDist2d(boss);
            if (distance < 34.0f)
            {
                return 1.0f;
            }
        }
        return 0.0f;
    }
    
    if (dynamic_cast<MurmurResonanceAction*>(action))
    {
        if (boss->HasAura(SPELL_RESONANCE) || boss->FindCurrentSpellBySpellId(SPELL_RESONANCE))
        {
            return 1.0f;
        }
        return 0.0f;
    }
    
    if (dynamic_cast<MurmurMagneticPullAction*>(action))
    {
        if (bot->HasAura(SPELL_MAGNETIC_PULL))
        {
            return 1.0f;
        }
        return 0.0f;
    }
    
    if (dynamic_cast<MurmurThunderingStormAction*>(action))
    {
        if (bot->GetMap()->IsHeroic() && boss->FindCurrentSpellBySpellId(SPELL_THUNDERING_STORM))
        {
            return 1.0f;
        }
        return 0.0f;
    }
    
    return 1.0f;
}