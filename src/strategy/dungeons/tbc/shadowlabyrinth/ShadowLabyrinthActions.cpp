#include "Playerbots.h"
#include "ShadowLabyrinthActions.h"
#include "ShadowLabyrinthStrategy.h"
#include "ShadowLabyrinthHelpers.h"

#include "Group.h"

#include <cmath>

namespace
{
void ClearVorpilCombatMovement(Player* bot)
{
    if (!bot)
        return;

    bot->AttackStop();
    bot->InterruptNonMeleeSpells(true);
    bot->StopMoving();
    if (MotionMaster* motionMaster = bot->GetMotionMaster())
        motionMaster->Clear();
}
}

bool AvoidCorrosiveAcidAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "ambassador hellmaw");
    if (!boss)
    {
        return false;
    }
    
    // Corrosive Acid is a frontal cone on random target, spread out
    if (boss->FindCurrentSpellBySpellId(SPELL_CORROSIVE_ACID))
    {
        // Check if other players are near us
        Group* group = bot->GetGroup();
        if (group)
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (member && member != bot && member->IsAlive())
                {
                    float dist = bot->GetExactDist2d(member);
                    if (dist < 8.0f)
                    {
                        // Move away from other players to avoid acid spread
                        float angle = bot->GetAngle(member) + M_PI;
                        float destX = bot->GetPositionX() + cos(angle) * 10.0f;
                        float destY = bot->GetPositionY() + sin(angle) * 10.0f;
                        return MoveTo(bot->GetMapId(), destX, destY, bot->GetPositionZ(),
                                    false, false, false, true, MovementPriority::MOVEMENT_FORCED);
                    }
                }
            }
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
    
    // Check if boss is casting Fear (33547) and spread preemptively
    if (boss->FindCurrentSpellBySpellId(SL_SPELL_FEAR))
    {
        // Spread out to minimize fear chains
        Group* group = bot->GetGroup();
        if (group)
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (member && member != bot && member->IsAlive())
                {
                    float dist = bot->GetExactDist2d(member);
                    if (dist < 10.0f)
                    {
                        float angle = bot->GetAngle(member) + M_PI;
                        float destX = bot->GetPositionX() + cos(angle) * 12.0f;
                        float destY = bot->GetPositionY() + sin(angle) * 12.0f;
                        return MoveTo(bot->GetMapId(), destX, destY, bot->GetPositionZ(),
                                    false, false, false, true, MovementPriority::MOVEMENT_FORCED);
                    }
                }
            }
        }
    }
    
    return false;
}

bool InciteChaosReactAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
        
    Unit* boss = AI_VALUE2(Unit*, "find target", "blackheart the inciter");
    if (!boss)
        return false;
    
    // During Incite Chaos, players attack each other AND boss becomes immune - spread out!
    if (boss->FindCurrentSpellBySpellId(SPELL_INCITE_CHAOS) || bot->HasAura(SPELL_INCITE_CHAOS_B))
    {
        // CRITICAL: Boss is immune during Incite Chaos - stop attacking boss
        if (boss->HasUnitFlag(UNIT_FLAG_IMMUNE_TO_PC) || boss->HasUnitFlag(UNIT_FLAG_IMMUNE_TO_NPC))
        {
            // Clear target if currently attacking immune boss to prevent wasted abilities
            Unit* currentTarget = AI_VALUE(Unit*, "current target");
            if (currentTarget && currentTarget->GetGUID() == boss->GetGUID())
            {
                botAI->ChangeStrategy("-follow,+stay", BotState::BOT_STATE_COMBAT);
                // Stop attacking boss during immunity - will resume after chaos ends
            }
        }
        // Move away from all other players to minimize damage
        float bestAngle = 0;
        float maxMinDist = 0;
        
        for (float angle = 0; angle < 2 * M_PI; angle += M_PI / 8)
        {
            float testX = boss->GetPositionX() + cos(angle) * 20.0f;
            float testY = boss->GetPositionY() + sin(angle) * 20.0f;
            float minDist = 100.0f;
            
            Group* group = bot->GetGroup();
            if (group)
            {
                for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
                {
                    Player* member = ref->GetSource();
                    if (member && member != bot && member->IsAlive())
                    {
                        float dist = member->GetExactDist2d(testX, testY);
                        if (dist < minDist)
                            minDist = dist;
                    }
                }
            }
            
            if (minDist > maxMinDist)
            {
                maxMinDist = minDist;
                bestAngle = angle;
            }
        }
        
        float destX = boss->GetPositionX() + cos(bestAngle) * 20.0f;
        float destY = boss->GetPositionY() + sin(bestAngle) * 20.0f;
        return MoveTo(bot->GetMapId(), destX, destY, bot->GetPositionZ(),
                    false, false, false, true, MovementPriority::MOVEMENT_FORCED);
    }
    
    return false;
}

bool AvoidWarStompAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
        
    Unit* boss = AI_VALUE2(Unit*, "find target", "blackheart the inciter");
    if (!boss)
        return false;
    
    // REACTIVE WAR STOMP AVOIDANCE: Move away when boss actually casts War Stomp
    if (boss->FindCurrentSpellBySpellId(SL_SPELL_WAR_STOMP))
    {
        float currentDistance = bot->GetExactDist2d(boss);
        if (currentDistance < 12.0f) // Within stomp range
        {
            // Move directly away from boss to avoid the stomp
            float angle = boss->GetAngle(bot) + M_PI; // Opposite direction
            float moveDistance = 15.0f - currentDistance; // Get outside 12+ yard range
            float destX = bot->GetPositionX() + cos(angle) * moveDistance;
            float destY = bot->GetPositionY() + sin(angle) * moveDistance;
            
            return MoveTo(bot->GetMapId(), destX, destY, bot->GetPositionZ(),
                        false, false, false, true, MovementPriority::MOVEMENT_FORCED);
        }
    }
    
    return false;
}

bool BlackheartChargeReactAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;
        
    Unit* boss = AI_VALUE2(Unit*, "find target", "blackheart the inciter");
    if (!boss)
        return false;
    
    // CHARGE REACTION: Boss charges at bot, need to prepare for impact and reposition
    if (boss->FindCurrentSpellBySpellId(SL_SPELL_CHARGE))
    {
        // Check if this bot is being charged
        if (boss->HasUnitState(UNIT_STATE_CASTING))
        {
            Spell* currentSpell = boss->GetCurrentSpell(CURRENT_GENERIC_SPELL);
            if (currentSpell && currentSpell->GetSpellInfo()->Id == SL_SPELL_CHARGE)
            {
                Unit* target = currentSpell->m_targets.GetUnitTarget();
                if (target && target->GetGUID() == bot->GetGUID())
                {
                    // Bot is being charged - notify group by moving slightly to indicate threat
                    // This helps tanks regain aggro after charge
                    
                    // Move perpendicular to boss to avoid being in direct line after charge
                    float angle = boss->GetAngle(bot) + (M_PI / 2.0f); // 90 degrees
                    float destX = bot->GetPositionX() + cos(angle) * 8.0f;
                    float destY = bot->GetPositionY() + sin(angle) * 8.0f;
                    
                    return MoveTo(bot->GetMapId(), destX, destY, bot->GetPositionZ(),
                                false, false, false, true, MovementPriority::MOVEMENT_FORCED);
                }
            }
        }
    }
    
    return false;
}

bool VorpilSpreadAction::isUseful()
{
    Player* bot = botAI->GetBot();
    if (!bot || !botAI || botAI->IsTank(bot))
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "grandmaster vorpil");
    if (!ShadowLabyrinth::IsGrandmasterVorpil(boss) || !boss->IsAlive() || !boss->IsInCombat())
        return false;

    if (boss->GetHealthPct() <= 12.0f)
        return false;

    if (ShadowLabyrinth::FindNearestVoidTravelerCached(botAI, bot, boss, 80.0f))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member != bot && member->IsAlive() && bot->GetExactDist2d(member) < 8.0f)
            return true;
    }

    return false;
}

bool VorpilSpreadAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Unit* boss = AI_VALUE2(Unit*, "find target", "grandmaster vorpil");
    if (!ShadowLabyrinth::IsGrandmasterVorpil(boss) || !boss->IsAlive() || !boss->IsInCombat())
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

    ShadowLabyrinth::VorpilCache& cache = ShadowLabyrinth::GetVorpilCache(bot->GetGUID());
    uint32 const now = getMSTime();
    Position destination = ShadowLabyrinth::GetVorpilSafeSpreadPosition(bot, boss, nearestAlly);
    if (!ShadowLabyrinth::ShouldIssueMovement(cache.lastMoveMs, cache.lastMovePos, destination, now, 500U, 3.0f))
        return false;

    return MoveTo(bot->GetMapId(), destination.GetPositionX(), destination.GetPositionY(), destination.GetPositionZ(),
        false, false, false, true, MovementPriority::MOVEMENT_FORCED);
}

bool MoveFromRainOfFireAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    Unit* boss = AI_VALUE2(Unit*, "find target", "grandmaster vorpil");
    if (!bot || !ShadowLabyrinth::IsGrandmasterVorpil(boss) || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    ShadowLabyrinth::VorpilCache& cache = ShadowLabyrinth::GetVorpilCache(bot->GetGUID());
    uint32 const now = getMSTime();
    if (boss->FindCurrentSpellBySpellId(SPELL_RAIN_OF_FIRE) || bot->HasAura(SPELL_RAIN_OF_FIRE) ||
        boss->FindCurrentSpellBySpellId(SPELL_DRAW_SHADOWS))
    {
        Position destination = ShadowLabyrinth::GetVorpilEscapePosition(bot, boss);
        if (!ShadowLabyrinth::ShouldIssueMovement(cache.lastMoveMs, cache.lastMovePos, destination, now, 250U, 2.0f))
        {
            return false;
        }

        ClearVorpilCombatMovement(bot);

        if (ShadowLabyrinth::IsVorpilOnUpperPlatform(bot) &&
            destination.GetPositionZ() + 1.0f < bot->GetPositionZ())
        {
            if (MoveTo(bot->GetMapId(), destination.GetPositionX(), destination.GetPositionY(), destination.GetPositionZ(),
                false, false, false, true, MovementPriority::MOVEMENT_FORCED, true))
            {
                return true;
            }

            return JumpTo(bot->GetMapId(), destination.GetPositionX(), destination.GetPositionY(), destination.GetPositionZ(),
                MovementPriority::MOVEMENT_FORCED);
        }

        return MoveTo(bot->GetMapId(), destination.GetPositionX(), destination.GetPositionY(), destination.GetPositionZ(),
            false, false, false, true, MovementPriority::MOVEMENT_FORCED, true);
    }

    return false;
}

bool DrawShadowsReactAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    Unit* boss = AI_VALUE2(Unit*, "find target", "grandmaster vorpil");
    if (!bot || !ShadowLabyrinth::IsGrandmasterVorpil(boss) || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    if (boss->FindCurrentSpellBySpellId(SPELL_DRAW_SHADOWS))
    {
        ShadowLabyrinth::VorpilCache& cache = ShadowLabyrinth::GetVorpilCache(bot->GetGUID());
        uint32 const now = getMSTime();
        Position destination = ShadowLabyrinth::GetVorpilEscapePosition(bot, boss);
        if (!ShadowLabyrinth::ShouldIssueMovement(cache.lastMoveMs, cache.lastMovePos, destination, now, 250U, 2.0f))
        {
            return false;
        }

        ClearVorpilCombatMovement(bot);

        if (ShadowLabyrinth::IsVorpilOnUpperPlatform(bot) &&
            destination.GetPositionZ() + 1.0f < bot->GetPositionZ())
        {
            if (MoveTo(bot->GetMapId(), destination.GetPositionX(), destination.GetPositionY(), destination.GetPositionZ(),
                false, false, false, true, MovementPriority::MOVEMENT_FORCED, true))
            {
                return true;
            }

            return JumpTo(bot->GetMapId(), destination.GetPositionX(), destination.GetPositionY(), destination.GetPositionZ(),
                MovementPriority::MOVEMENT_FORCED);
        }

        return MoveTo(bot->GetMapId(), destination.GetPositionX(), destination.GetPositionY(), destination.GetPositionZ(),
            false, false, false, true, MovementPriority::MOVEMENT_FORCED, true);
    }
    
    return false;
}

bool MurmurSonicBoomAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "murmur");
    Player* bot = botAI->GetBot();
    if (!bot || !ShadowLabyrinth::IsMurmur(boss) || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }

    ShadowLabyrinth::MurmurCache& cache = ShadowLabyrinth::GetMurmurCache(bot->GetGUID());
    uint32 const now = getMSTime();
    bool const sonicBoomCasting = ShadowLabyrinth::IsMurmurCastingSonicBoom(boss);
    if (sonicBoomCasting)
    {
        cache.sonicBoomDangerUntilMs = now + 4000U;
        cache.sonicBoomReturnUntilMs = now + 7000U;
    }

    float const currentDist = bot->GetExactDist(boss);
    float const safeDistance = 48.0f;
    if (cache.sonicBoomDangerUntilMs > now)
    {
        if (currentDist < safeDistance)
        {
            Position destination = ShadowLabyrinth::GetMurmurSafeMovePosition(bot, boss, nullptr, safeDistance);
            if (ShadowLabyrinth::ShouldIssueMovement(cache.lastMoveMs, cache.lastMovePos, destination, now, 500U, 3.0f))
            {
                return MoveTo(boss->GetMapId(), destination.GetPositionX(), destination.GetPositionY(), destination.GetPositionZ(),
                    false, false, false, true, MovementPriority::MOVEMENT_FORCED);
            }
        }
        return false;
    }

    if (cache.sonicBoomReturnUntilMs > now)
    {
        float const desiredRange = botAI->IsTank(bot) || botAI->IsMelee(bot) ? 5.0f : 28.0f;
        if (std::fabs(currentDist - desiredRange) > 3.0f)
        {
            Position destination = ShadowLabyrinth::GetMurmurSafeMovePosition(bot, boss, nullptr, desiredRange);
            if (ShadowLabyrinth::ShouldIssueMovement(cache.lastMoveMs, cache.lastMovePos, destination, now, 500U, 3.0f))
            {
                return MoveTo(boss->GetMapId(), destination.GetPositionX(), destination.GetPositionY(), destination.GetPositionZ(),
                    false, false, false, true, MovementPriority::MOVEMENT_FORCED);
            }
        }
        else
        {
            cache.sonicBoomReturnUntilMs = 0;
        }
    }
    
    return false;
}

bool MurmurResonanceAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "murmur");
    Player* bot = botAI->GetBot();
    if (!bot || !ShadowLabyrinth::IsMurmur(boss) || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    if (botAI->IsTank(bot))
    {
        if (bot->GetExactDist(boss) > 5.0f)
        {
            ShadowLabyrinth::MurmurCache& cache = ShadowLabyrinth::GetMurmurCache(bot->GetGUID());
            uint32 const now = getMSTime();
            Position destination = ShadowLabyrinth::GetMurmurSafeMovePosition(bot, boss, nullptr, 5.0f);
            if (!ShadowLabyrinth::ShouldIssueMovement(cache.lastMoveMs, cache.lastMovePos, destination, now, 500U, 3.0f))
                return false;

            return MoveTo(boss->GetMapId(), destination.GetPositionX(), destination.GetPositionY(), destination.GetPositionZ(),
                false, false, false, true, MovementPriority::MOVEMENT_FORCED);
        }
    }
    else if (boss->FindCurrentSpellBySpellId(SPELL_RESONANCE) && bot->GetExactDist(boss) < 10.0f)
    {
        Group* group = bot->GetGroup();
        if (group)
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (member && member != bot && member->IsAlive() && bot->GetExactDist2d(member) < 10.0f)
                {
                    float angle = bot->GetAngle(member) + M_PI;
                    float destX = bot->GetPositionX() + cos(angle) * 12.0f;
                    float destY = bot->GetPositionY() + sin(angle) * 12.0f;
                    ShadowLabyrinth::MurmurCache& cache = ShadowLabyrinth::GetMurmurCache(bot->GetGUID());
                    uint32 const now = getMSTime();
                    Position destination = {destX, destY, bot->GetPositionZ()};
                    if (!ShadowLabyrinth::ShouldIssueMovement(cache.lastMoveMs, cache.lastMovePos, destination, now, 500U, 3.0f))
                        return false;

                    return MoveTo(bot->GetMapId(), destX, destY, bot->GetPositionZ(),
                        false, false, false, true, MovementPriority::MOVEMENT_FORCED);
                }
            }
        }
    }
    
    return false;
}

bool MurmurMagneticPullAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "murmur");
    Player* bot = botAI->GetBot();
    if (!bot || !ShadowLabyrinth::IsMurmur(boss) || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    ShadowLabyrinth::MurmurCache& cache = ShadowLabyrinth::GetMurmurCache(bot->GetGUID());
    uint32 const now = getMSTime();
    Player* touchedPlayer = ShadowLabyrinth::FindTouchedPlayerCached(botAI, bot, 18.0f);
    bool const selfTouched = ShadowLabyrinth::HasMurmursTouch(bot) || bot->HasAura(SL_SPELL_MAGNETIC_PULL);

    if (selfTouched || touchedPlayer)
    {
        Unit const* threatSource = nullptr;
        if (selfTouched)
        {
            Group* group = bot->GetGroup();
            if (group)
            {
                for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
                {
                    Player* member = ref->GetSource();
                    if (member && member != bot && member->IsAlive() && !ShadowLabyrinth::HasMurmursTouch(member) &&
                        bot->GetExactDist2d(member) < 15.0f)
                    {
                        threatSource = member;
                        break;
                    }
                }
            }

            if (!threatSource)
                threatSource = boss;
        }

        if (!threatSource)
            threatSource = touchedPlayer;

        Position destination = ShadowLabyrinth::GetMurmurSafeMovePosition(bot, boss, threatSource, 20.0f);
        if (ShadowLabyrinth::ShouldIssueMovement(cache.lastMoveMs, cache.lastMovePos, destination, now, 500U, 3.0f))
        {
            cache.touchDangerUntilMs = now + 2500U;
            cache.touchReturnUntilMs = now + 5000U;
            return MoveTo(bot->GetMapId(), destination.GetPositionX(), destination.GetPositionY(), destination.GetPositionZ(),
                false, false, false, true, MovementPriority::MOVEMENT_FORCED);
        }

        return false;
    }

    if (cache.touchReturnUntilMs > now)
    {
        float const desiredRange = botAI->IsTank(bot) || botAI->IsMelee(bot) ? 5.0f : (botAI->IsHeal(bot) ? 28.0f : 24.0f);
        if (bot->GetExactDist(boss) > desiredRange + 3.0f)
        {
            Position destination = ShadowLabyrinth::GetMurmurSafeMovePosition(bot, boss, nullptr, desiredRange);
            if (ShadowLabyrinth::ShouldIssueMovement(cache.lastMoveMs, cache.lastMovePos, destination, now, 500U, 3.0f))
            {
                return MoveTo(bot->GetMapId(), destination.GetPositionX(), destination.GetPositionY(), destination.GetPositionZ(),
                    false, false, false, true, MovementPriority::MOVEMENT_FORCED);
            }
        }
        else
        {
            cache.touchReturnUntilMs = 0;
        }
    }
    
    return false;
}

bool MurmurThunderingStormAction::Execute(Event event)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "murmur");
    Player* bot = botAI->GetBot();
    if (!bot || !ShadowLabyrinth::IsMurmur(boss) || !boss->IsAlive() || !boss->IsInCombat())
    {
        return false;
    }
    
    if (boss->FindCurrentSpellBySpellId(SPELL_THUNDERING_STORM))
    {
        float const currentDist = bot->GetExactDist(boss);
        if (currentDist >= 25.0f && currentDist <= 100.0f)
        {
            ShadowLabyrinth::MurmurCache& cache = ShadowLabyrinth::GetMurmurCache(bot->GetGUID());
            uint32 const now = getMSTime();
            if (botAI->IsTank(bot) || botAI->IsMelee(bot))
            {
                Position destination = ShadowLabyrinth::GetMurmurSafeMovePosition(bot, boss, nullptr, 5.0f);
                if (!ShadowLabyrinth::ShouldIssueMovement(cache.lastMoveMs, cache.lastMovePos, destination, now, 500U, 3.0f))
                    return false;

                return MoveTo(boss->GetMapId(), destination.GetPositionX(), destination.GetPositionY(), destination.GetPositionZ(),
                    false, false, false, true, MovementPriority::MOVEMENT_FORCED);
            }
            else
            {
                float angle = boss->GetAngle(bot) + M_PI;
                Position destination = {boss->GetPositionX() + cos(angle) * 110.0f,
                    boss->GetPositionY() + sin(angle) * 110.0f, bot->GetPositionZ()};
                if (!ShadowLabyrinth::ShouldIssueMovement(cache.lastMoveMs, cache.lastMovePos, destination, now, 500U, 3.0f))
                    return false;

                return MoveTo(bot->GetMapId(), destination.GetPositionX(), destination.GetPositionY(), destination.GetPositionZ(),
                    false, false, false, true, MovementPriority::MOVEMENT_FORCED);
            }
        }
    }
    
    return false;
}
