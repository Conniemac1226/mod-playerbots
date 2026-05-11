#include "Playerbots.h"
#include "ShadowLabyrinthMultipliers.h"
#include "ShadowLabyrinthActions.h"
#include "ShadowLabyrinthHelpers.h"

#include "Group.h"
#include <algorithm>

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
            return 1.0f;
        return 0.0f;
    }

    if (dynamic_cast<HellmawFearReactAction*>(action))
    {
        if (boss->FindCurrentSpellBySpellId(SL_SPELL_FEAR))
            return 1.0f;
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
            return 1.0f;
        return 0.0f;
    }

    if (dynamic_cast<AvoidWarStompAction*>(action))
    {
        if (boss->FindCurrentSpellBySpellId(SL_SPELL_WAR_STOMP))
        {
            float const distance = bot->GetExactDist2d(boss);
            if (distance < 10.0f)
                return 1.0f;
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

    Player* player = botAI->GetBot();
    if (!player)
        return 1.0f;

    if (dynamic_cast<VoidTravelerPriorityAction*>(action))
    {
        if (botAI->IsHeal(player))
            return 0.0f;

        Unit* voidTraveler = ShadowLabyrinth::FindNearestVoidTravelerCached(botAI, player, boss, 80.0f);
        if (!voidTraveler)
            return 0.0f;

        if (botAI->IsTank(player))
            return voidTraveler->GetDistance(boss) < 20.0f ? 1.0f : 0.0f;

        return boss->GetHealthPct() > 12.0f ? 1.0f : 0.0f;
    }

    if (dynamic_cast<VorpilSpreadAction*>(action))
    {
        if (botAI->IsTank(player) || boss->GetHealthPct() <= 12.0f)
            return 0.0f;

        if (ShadowLabyrinth::FindNearestVoidTravelerCached(botAI, player, boss, 80.0f))
            return 0.0f;

        Group* group = player->GetGroup();
        if (!group)
            return 0.0f;

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member != player && member->IsAlive() && player->GetExactDist2d(member) < 8.0f)
                return 1.0f;
        }

        return 0.0f;
    }

    if (dynamic_cast<MoveFromRainOfFireAction*>(action))
    {
        if (boss->HasAura(SPELL_RAIN_OF_FIRE) || boss->FindCurrentSpellBySpellId(SPELL_RAIN_OF_FIRE))
        {
            Position centerPos = ShadowLabyrinth::GetVorpilCenter();
            if (player->GetExactDist2d(centerPos.GetPositionX(), centerPos.GetPositionY()) < 12.0f)
                return 1.0f;
        }
        return 0.0f;
    }

    if (dynamic_cast<DrawShadowsReactAction*>(action))
    {
        if (boss->HasAura(SPELL_DRAW_SHADOWS) || boss->FindCurrentSpellBySpellId(SPELL_DRAW_SHADOWS))
            return 1.0f;
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

    Player* player = botAI->GetBot();
    if (!player)
        return 1.0f;

    if (dynamic_cast<MurmurSonicBoomAction*>(action))
    {
        ShadowLabyrinth::MurmurCache& cache = ShadowLabyrinth::GetMurmurCache(player->GetGUID());
        uint32 const now = getMSTime();

        if (ShadowLabyrinth::IsMurmurCastingSonicBoom(boss))
        {
            cache.sonicBoomDangerUntilMs = now + 4000U;
            cache.sonicBoomReturnUntilMs = now + 7000U;
        }

        if (cache.sonicBoomDangerUntilMs > now)
            return player->GetExactDist(boss) < 48.0f ? 1.0f : 0.0f;

        return cache.sonicBoomReturnUntilMs > now ? 1.0f : 0.0f;
    }

    if (dynamic_cast<MurmurResonanceAction*>(action))
    {
        if (!boss->FindCurrentSpellBySpellId(SPELL_RESONANCE))
            return 0.0f;

        return botAI->IsTank(player) ? 1.0f : (player->GetExactDist(boss) < 10.0f ? 1.0f : 0.0f);
    }

    if (dynamic_cast<MurmurMagneticPullAction*>(action))
    {
        ShadowLabyrinth::MurmurCache& cache = ShadowLabyrinth::GetMurmurCache(player->GetGUID());
        uint32 const now = getMSTime();

        if (ShadowLabyrinth::HasMurmursTouch(player) || player->HasAura(SL_SPELL_MAGNETIC_PULL))
        {
            Aura* touch = player->GetAura(SPELL_MURMURS_TOUCH);
            if (!touch)
                touch = player->GetAura(SPELL_MURMURS_TOUCH_HEROIC);

            if (touch)
            {
                cache.touchDangerUntilMs = now + static_cast<uint32>(std::max(0, touch->GetDuration()));
                cache.touchReturnUntilMs = cache.touchDangerUntilMs + 2000U;
            }

            return 1.0f;
        }

        if (ShadowLabyrinth::FindTouchedPlayerCached(botAI, player, 18.0f))
        {
            cache.touchReturnUntilMs = now + 3500U;
            return 1.0f;
        }

        return cache.touchReturnUntilMs > now ? 1.0f : 0.0f;
    }

    if (dynamic_cast<MurmurThunderingStormAction*>(action))
    {
        if (!player->GetMap()->IsHeroic() || !boss->FindCurrentSpellBySpellId(SPELL_THUNDERING_STORM))
            return 0.0f;

        float const distance = player->GetExactDist(boss);
        return (distance >= 25.0f && distance <= 100.0f) ? 1.0f : 0.0f;
    }

    return 1.0f;
}

float VorpilVoidTravelerMultiplier::GetValue(Action* action)
{
    if (!action || action->getName() != "dps assist")
        return 1.0f;

    Player* bot = botAI->GetBot();
    Unit* boss = AI_VALUE2(Unit*, "find target", "grandmaster vorpil");
    if (!bot || !ShadowLabyrinth::IsGrandmasterVorpil(boss) || !boss->IsAlive() || !boss->IsInCombat())
        return 1.0f;

    if (botAI->IsTank(bot))
        return 1.0f;

    Unit* voidTraveler = ShadowLabyrinth::FindNearestVoidTravelerCached(botAI, bot, boss, 80.0f);
    if (!voidTraveler)
        return 1.0f;

    return boss->GetHealthPct() > 12.0f ? 0.0f : 1.0f;
}
