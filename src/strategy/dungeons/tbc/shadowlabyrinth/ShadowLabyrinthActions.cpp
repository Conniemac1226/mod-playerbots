#include "Playerbots.h"
#include "ShadowLabyrinthActions.h"
#include "ShadowLabyrinthStrategy.h"

bool AvoidCorrosiveAcidAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "ambassador hellmaw");
    if (!boss)
    {
        return false;
    }
    
    if (bot->HasAura(SPELL_CORROSIVE_ACID))
    {
        return false;
    }
    
    if (boss->FindCurrentSpellBySpellId(SPELL_CORROSIVE_ACID))
    {
        Unit* target = boss->GetVictim();
        if (target && target == bot)
        {
            float safeDistance = 10.0f;
            return MoveAway(boss, safeDistance);
        }
    }
    
    return false;
}

bool HellmawFearReactAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "ambassador hellmaw");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    if (bot->HasAura(SPELL_FEAR))
    {
        return false;
    }
    
    return false;
}

bool InciteChaosReactAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "blackheart the inciter");
    if (!boss)
    {
        return false;
    }
    
    if (bot->HasAura(SPELL_INCITE_CHAOS_B))
    {
        return false;
    }
    
    return false;
}

bool AvoidWarStompAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "blackheart the inciter");
    if (!boss)
    {
        return false;
    }
    
    if (boss->FindCurrentSpellBySpellId(SPELL_WAR_STOMP))
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

bool VoidTravelerPriorityAction::isUseful()
{
    return !botAI->IsHeal(bot);
}

bool VoidTravelerPriorityAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "grandmaster vorpil");
    if (!boss || !boss->IsInCombat())
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
        
        if (unit->GetEntry() == NPC_VOID_TRAVELER)
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

bool MoveFromRainOfFireAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "grandmaster vorpil");
    if (!boss)
    {
        return false;
    }
    
    if (boss->HasAura(SPELL_RAIN_OF_FIRE) || boss->FindCurrentSpellBySpellId(SPELL_RAIN_OF_FIRE))
    {
        Position centerPos = {-253.548f, -263.646f, 17.0864f};
        float currentDist = bot->GetExactDist2d(centerPos.GetPositionX(), centerPos.GetPositionY());
        
        if (currentDist < 12.0f)
        {
            float moveDistance = 14.0f - currentDist;
            float angle = bot->GetAngle(&centerPos);
            float destX = bot->GetPositionX() - cos(angle) * moveDistance;
            float destY = bot->GetPositionY() - sin(angle) * moveDistance;
            return MoveTo(bot->GetMapId(), destX, destY, bot->GetPositionZ());
        }
    }
    
    return false;
}

bool DrawShadowsReactAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "grandmaster vorpil");
    if (!boss)
    {
        return false;
    }
    
    if (boss->HasAura(SPELL_DRAW_SHADOWS) || boss->FindCurrentSpellBySpellId(SPELL_DRAW_SHADOWS))
    {
        Position centerPos = {-253.548f, -263.646f, 17.0864f};
        float safeDistance = 30.0f;
        float currentDist = bot->GetExactDist2d(centerPos.GetPositionX(), centerPos.GetPositionY());
        
        if (currentDist < safeDistance)
        {
            float moveDistance = safeDistance - currentDist + 5.0f;
            float angle = bot->GetAngle(&centerPos);
            float destX = bot->GetPositionX() - cos(angle) * moveDistance;
            float destY = bot->GetPositionY() - sin(angle) * moveDistance;
            return MoveTo(bot->GetMapId(), destX, destY, bot->GetPositionZ());
        }
    }
    
    return false;
}

bool MurmurSonicBoomAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "murmur");
    if (!boss)
    {
        return false;
    }
    
    if (boss->HasAura(SPELL_SONIC_BOOM_CAST) || boss->FindCurrentSpellBySpellId(SPELL_SONIC_BOOM_CAST))
    {
        float safeDistance = 34.0f;
        float currentDist = bot->GetExactDist2d(boss);
        
        if (currentDist < safeDistance)
        {
            return MoveAway(boss, safeDistance - currentDist + 2.0f);
        }
    }
    
    return false;
}

bool MurmurResonanceAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "murmur");
    if (!boss)
    {
        return false;
    }
    
    if (boss->HasAura(SPELL_RESONANCE) || boss->FindCurrentSpellBySpellId(SPELL_RESONANCE))
    {
        if (botAI->IsTank(bot))
        {
            float targetDist = 5.0f;
            float currentDist = bot->GetExactDist2d(boss);
            
            if (currentDist > targetDist)
            {
                return MoveTo(boss->GetMapId(), boss->GetPositionX(), boss->GetPositionY(), boss->GetPositionZ());
            }
        }
    }
    
    return false;
}

bool MurmurMagneticPullAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "murmur");
    if (!boss)
    {
        return false;
    }
    
    if (bot->HasAura(SPELL_MAGNETIC_PULL))
    {
        float safeDistance = 15.0f;
        float currentDist = bot->GetExactDist2d(boss);
        
        if (currentDist < safeDistance)
        {
            return MoveAway(boss, safeDistance - currentDist + 2.0f);
        }
    }
    
    return false;
}

bool MurmurThunderingStormAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "murmur");
    if (!boss)
    {
        return false;
    }
    
    if (boss->FindCurrentSpellBySpellId(SPELL_THUNDERING_STORM))
    {
        float safeDistance = 15.0f;
        float currentDist = bot->GetExactDist2d(boss);
        
        if (currentDist < safeDistance && !botAI->IsTank(bot))
        {
            return MoveAway(boss, safeDistance - currentDist + 2.0f);
        }
    }
    
    return false;
}