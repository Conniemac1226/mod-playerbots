#include "Playerbots.h"
#include "ShadowLabyrinthTriggers.h"
#include "ShadowLabyrinthActions.h"

bool HellmawCorrosiveAcidTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "ambassador hellmaw");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    return boss->FindCurrentSpellBySpellId(SPELL_CORROSIVE_ACID) && boss->GetVictim() == bot;
}

bool HellmawFearTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "ambassador hellmaw");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    return boss->FindCurrentSpellBySpellId(SPELL_FEAR);
}

bool BlackheartInciteChaosTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "blackheart the inciter");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    return boss->FindCurrentSpellBySpellId(SPELL_INCITE_CHAOS) || bot->HasAura(SPELL_INCITE_CHAOS_B);
}

bool BlackheartWarStompTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "blackheart the inciter");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    if (boss->FindCurrentSpellBySpellId(SPELL_WAR_STOMP))
    {
        float distance = bot->GetExactDist2d(boss);
        return distance < 10.0f;
    }
    
    return false;
}

bool VorpilVoidTravelerTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "grandmaster vorpil");
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
        
        if (unit->GetEntry() == NPC_VOID_TRAVELER)
        {
            return true;
        }
    }
    
    return false;
}

bool VorpilRainOfFireTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "grandmaster vorpil");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    if (boss->HasAura(SPELL_RAIN_OF_FIRE) || boss->FindCurrentSpellBySpellId(SPELL_RAIN_OF_FIRE))
    {
        Position centerPos = {-253.548f, -263.646f, 17.0864f};
        float distance = bot->GetExactDist2d(centerPos.GetPositionX(), centerPos.GetPositionY());
        return distance < 12.0f;
    }
    
    return false;
}

bool VorpilDrawShadowsTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "grandmaster vorpil");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    return boss->HasAura(SPELL_DRAW_SHADOWS) || boss->FindCurrentSpellBySpellId(SPELL_DRAW_SHADOWS);
}

bool MurmurSonicBoomTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "murmur");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    if (boss->HasAura(SPELL_SONIC_BOOM_CAST) || boss->FindCurrentSpellBySpellId(SPELL_SONIC_BOOM_CAST))
    {
        float distance = bot->GetExactDist2d(boss);
        return distance < 34.0f;
    }
    
    return false;
}

bool MurmurResonanceTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "murmur");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    return boss->HasAura(SPELL_RESONANCE) || boss->FindCurrentSpellBySpellId(SPELL_RESONANCE);
}

bool MurmurMagneticPullTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "murmur");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    return bot->HasAura(SPELL_MAGNETIC_PULL);
}

bool MurmurThunderingStormTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "murmur");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    if (bot->GetMap()->IsHeroic())
    {
        return boss->FindCurrentSpellBySpellId(SPELL_THUNDERING_STORM);
    }
    
    return false;
}