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
    
    return boss->FindCurrentSpellBySpellId(SL_SPELL_FEAR);
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
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
        
    Unit* boss = AI_VALUE2(Unit*, "find target", "blackheart the inciter");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;
    
    // REACTIVE DETECTION: Respond to actual War Stomp cast
    if (boss->FindCurrentSpellBySpellId(SL_SPELL_WAR_STOMP))
    {
        float distance = bot->GetExactDist2d(boss);
        return distance < 12.0f; // War Stomp affects ~10 yard radius, safety margin
    }
    
    return false;
}

bool BlackheartChargeTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
        
    Unit* boss = AI_VALUE2(Unit*, "find target", "blackheart the inciter");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;
    
    // CHARGE DETECTION: Boss targets random player and charges at them
    // Detect if boss is casting Charge at this bot or if bot was recently charged
    if (boss->FindCurrentSpellBySpellId(SL_SPELL_CHARGE))
    {
        // Check if this bot is the target of the charge
        Unit* chargeTarget = boss->GetVictim();
        if (chargeTarget && chargeTarget->GetGUID() == bot->GetGUID())
            return true;
            
        // Also check by spell target (more reliable)
        if (boss->HasUnitState(UNIT_STATE_CASTING))
        {
            Spell* currentSpell = boss->GetCurrentSpell(CURRENT_GENERIC_SPELL);
            if (currentSpell && currentSpell->GetSpellInfo()->Id == SL_SPELL_CHARGE)
            {
                Unit* target = currentSpell->m_targets.GetUnitTarget();
                return (target && target->GetGUID() == bot->GetGUID());
            }
        }
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
    
    return bot->HasAura(SL_SPELL_MAGNETIC_PULL);
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