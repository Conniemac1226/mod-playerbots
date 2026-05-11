#include "Playerbots.h"
#include "ShadowLabyrinthTriggers.h"
#include "ShadowLabyrinthActions.h"
#include "ShadowLabyrinthHelpers.h"

#include "Group.h"

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
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
        
    Unit* boss = AI_VALUE2(Unit*, "find target", "grandmaster vorpil");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    return ShadowLabyrinth::FindNearestVoidTravelerCached(botAI, bot, boss, 80.0f) != nullptr;
}

bool VorpilSpreadTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot || botAI->IsTank(bot))
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "grandmaster vorpil");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    if (boss->GetHealthPct() <= 12.0f)
        return false;

    Player* nearestAlly = nullptr;
    float nearestDistance = 8.0f;
    Group* group = bot->GetGroup();
    if (group)
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || member == bot || !member->IsAlive())
                continue;

            float const distance = bot->GetExactDist2d(member);
            if (distance < nearestDistance)
            {
                nearestDistance = distance;
                nearestAlly = member;
            }
        }
    }

    if (!nearestAlly)
        return false;

    if (ShadowLabyrinth::FindNearestVoidTravelerCached(botAI, bot, boss, 80.0f))
        return false;

    return true;
}

bool VorpilRainOfFireTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "grandmaster vorpil");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    return boss->HasAura(SPELL_RAIN_OF_FIRE) || boss->FindCurrentSpellBySpellId(SPELL_RAIN_OF_FIRE);
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
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "murmur");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat() || !ShadowLabyrinth::IsMurmur(boss))
        return false;

    ShadowLabyrinth::MurmurCache& cache = ShadowLabyrinth::GetMurmurCache(bot->GetGUID());
    uint32 const now = getMSTime();
    if (ShadowLabyrinth::IsMurmurCastingSonicBoom(boss))
    {
        cache.sonicBoomDangerUntilMs = now + 4000U;
        cache.sonicBoomReturnUntilMs = now + 7000U;
    }

    if (cache.sonicBoomDangerUntilMs > now)
        return bot->GetExactDist2d(boss) < 48.0f;

    return cache.sonicBoomReturnUntilMs > now;
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
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "murmur");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    ShadowLabyrinth::MurmurCache& cache = ShadowLabyrinth::GetMurmurCache(bot->GetGUID());
    uint32 const now = getMSTime();
    Player* touchedPlayer = ShadowLabyrinth::FindTouchedPlayerCached(botAI, bot, 18.0f);
    if (touchedPlayer)
    {
        cache.touchReturnUntilMs = now + 3500U;
        return true;
    }

    if (bot->HasAura(SL_SPELL_MAGNETIC_PULL) || ShadowLabyrinth::HasMurmursTouch(bot))
    {
        Aura* touch = bot->GetAura(SPELL_MURMURS_TOUCH);
        if (!touch)
            touch = bot->GetAura(SPELL_MURMURS_TOUCH_HEROIC);

        if (touch)
        {
            cache.touchDangerUntilMs = std::max(cache.touchDangerUntilMs, now + static_cast<uint32>(std::max(0, touch->GetDuration())));
            cache.touchReturnUntilMs = std::max(cache.touchReturnUntilMs, cache.touchDangerUntilMs + 2000U);
        }

        return true;
    }

    return cache.touchReturnUntilMs > now;
}

bool MurmurThunderingStormTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

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
