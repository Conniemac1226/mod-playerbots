/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "OutlandWorldBossStrategy.h"

#include "AttackAction.h"
#include "Group.h"
#include "GenericSpellActions.h"
#include "Playerbots.h"
#include "Unit.h"

namespace
{
constexpr float DOOMWALKER_SPREAD_DISTANCE = 10.0f;
constexpr float DOOMWALKER_MELEE_RING_BUFFER = 1.0f;
constexpr uint8 DOOMWALKER_MELEE_CANDIDATES = 16;

bool HasGroupMemberWithinDistance(Player* bot, float distance)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    uint32 const mapId = bot->GetMapId();
    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* member = gref->GetSource();
        if (!member || member == bot || !member->IsAlive() || member->GetMapId() != mapId)
            continue;

        if (bot->GetDistance2d(member) < distance)
            return true;
    }

    return false;
}

// Melee stay on Doomwalker's boss ring and choose the spot that is farthest from the group.
bool FindDoomwalkerMeleeSpreadPosition(Player* bot, Unit* doomwalker,
                                       float spreadDistance,
                                       Position& destination)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> members;
    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* member = gref->GetSource();
        if (!member || member == bot || !member->IsAlive() ||
            member->GetMapId() != bot->GetMapId())
            continue;

        members.push_back(member);
    }

    if (members.empty())
        return false;

    float const bossReach = doomwalker->GetCombatReach();
    float const botReach = bot->GetCombatReach();
    float const meleeMoveDistance = bossReach + DOOMWALKER_MELEE_RING_BUFFER;
    float const meleeRange = bot->GetMeleeRange(doomwalker);

    bool foundDesired = false;
    bool foundFallback = false;
    float bestDesiredScore = -1.0f;
    float bestFallbackScore = -1.0f;
    Position bestDesiredPosition;
    Position bestFallbackPosition;

    for (uint8 i = 0; i < DOOMWALKER_MELEE_CANDIDATES; ++i)
    {
        float const angle = static_cast<float>(
            2.0f * M_PI * i / DOOMWALKER_MELEE_CANDIDATES);
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;

        doomwalker->GetNearPoint(
            bot, x, y, z, botReach, meleeMoveDistance, angle);

        if (!bot->GetMap()->CheckCollisionAndGetValidCoords(
                bot, bot->GetPositionX(), bot->GetPositionY(),
                bot->GetPositionZ(), x, y, z))
        {
            continue;
        }

        Position candidate(x, y, z);
        float const candidateX = candidate.GetPositionX();
        float const candidateY = candidate.GetPositionY();
        if (candidate.GetExactDist2d(doomwalker->GetPositionX(),
                                     doomwalker->GetPositionY()) >= meleeRange)
            continue;

        float nearestMemberDistance = 0.0f;
        bool hasMember = false;
        for (Player* member : members)
        {
            float const memberDistance =
                member->GetExactDist2d(candidateX, candidateY);
            if (!hasMember || memberDistance < nearestMemberDistance)
            {
                nearestMemberDistance = memberDistance;
                hasMember = true;
            }
        }

        if (!hasMember)
            return false;

        if (nearestMemberDistance >= spreadDistance)
        {
            if (!foundDesired || nearestMemberDistance > bestDesiredScore)
            {
                bestDesiredScore = nearestMemberDistance;
                bestDesiredPosition = candidate;
                foundDesired = true;
            }
        }
        else if (!foundDesired &&
                 (!foundFallback || nearestMemberDistance > bestFallbackScore))
        {
            bestFallbackScore = nearestMemberDistance;
            bestFallbackPosition = candidate;
            foundFallback = true;
        }
    }

    if (foundDesired)
    {
        destination = bestDesiredPosition;
        return true;
    }

    if (foundFallback)
    {
        destination = bestFallbackPosition;
        return true;
    }

    return false;
}

bool IsDoomwalkerCasting(Unit* doomwalker, uint32 spellId)
{
    return doomwalker && doomwalker->IsAlive() && doomwalker->IsInCombat() &&
           doomwalker->HasUnitState(UNIT_STATE_CASTING) &&
           doomwalker->FindCurrentSpellBySpellId(spellId);
}

bool IsOffensiveSpellAction(Action* action)
{
    if (!action || dynamic_cast<CastHealingSpellAction*>(action))
        return false;

    CastSpellAction* castSpellAction = dynamic_cast<CastSpellAction*>(action);
    if (!castSpellAction)
        return false;

    return castSpellAction->getThreatType() == Action::ActionThreatType::Single ||
           castSpellAction->getThreatType() == Action::ActionThreatType::Aoe;
}
}

bool DoomwalkerChainLightningSpreadAction::Execute(Event /*event*/)
{
    if (botAI->IsTank(bot))
        return false;

    Unit* doomwalker = AI_VALUE2(Unit*, "find target", "doomwalker");
    if (!IsDoomwalkerCasting(doomwalker, SPELL_DOOMWALKER_CHAIN_LIGHTNING))
        return false;

    if (!HasGroupMemberWithinDistance(bot, DOOMWALKER_SPREAD_DISTANCE))
        return false;

    bot->AttackStop();
    bot->InterruptNonMeleeSpells(true);
    if (botAI->IsMelee(bot))
    {
        Position spreadPosition;
        if (FindDoomwalkerMeleeSpreadPosition(bot, doomwalker,
                                              DOOMWALKER_SPREAD_DISTANCE,
                                              spreadPosition))
        {
            return MoveTo(bot->GetMapId(), spreadPosition.GetPositionX(),
                          spreadPosition.GetPositionY(),
                          spreadPosition.GetPositionZ(), false, false, false,
                          false,
                          MovementPriority::MOVEMENT_COMBAT, true);
        }

        if (!bot->IsWithinMeleeRange(doomwalker))
            return MoveFromGroup(DOOMWALKER_SPREAD_DISTANCE);

        return false;
    }

    return MoveFromGroup(DOOMWALKER_SPREAD_DISTANCE);
}

bool DoomwalkerChainLightningSpreadAction::isUseful()
{
    if (botAI->IsTank(bot))
        return false;

    Unit* doomwalker = AI_VALUE2(Unit*, "find target", "doomwalker");
    return IsDoomwalkerCasting(doomwalker, SPELL_DOOMWALKER_CHAIN_LIGHTNING) &&
           HasGroupMemberWithinDistance(bot, DOOMWALKER_SPREAD_DISTANCE);
}

bool DoomwalkerEarthquakeMoveAwayAction::Execute(Event /*event*/)
{
    if (botAI->IsTank(bot))
        return false;

    Unit* doomwalker = AI_VALUE2(Unit*, "find target", "doomwalker");
    if (!doomwalker || !doomwalker->IsAlive() || !doomwalker->IsInCombat() ||
        !doomwalker->HasUnitState(UNIT_STATE_CASTING) ||
        !doomwalker->FindCurrentSpellBySpellId(SPELL_DOOMWALKER_EARTHQUAKE))
        return false;

    float currentDistance = bot->GetExactDist2d(doomwalker);
    constexpr float safeDistance = 25.0f;
    if (currentDistance >= safeDistance)
        return false;

    bot->AttackStop();
    bot->InterruptNonMeleeSpells(true);
    return MoveAway(doomwalker, safeDistance - currentDistance);
}

bool DoomwalkerEarthquakeMoveAwayAction::isUseful()
{
    if (botAI->IsTank(bot))
        return false;

    Unit* doomwalker = AI_VALUE2(Unit*, "find target", "doomwalker");
    if (!doomwalker || !doomwalker->IsAlive() || !doomwalker->IsInCombat() ||
        !doomwalker->HasUnitState(UNIT_STATE_CASTING) ||
        !doomwalker->FindCurrentSpellBySpellId(SPELL_DOOMWALKER_EARTHQUAKE))
        return false;

    return bot->GetExactDist2d(doomwalker) < 25.0f;
}

bool DoomLordKazzakMoveAwayFromMarkAction::Execute(Event /*event*/)
{
    Unit* kazzak = AI_VALUE2(Unit*, "find target", "doom lord kazzak");
    if (!kazzak || !kazzak->IsAlive() || !kazzak->IsInCombat() ||
        !bot->HasAura(SPELL_DOOM_LORD_KAZZAK_MARK_OF_KAZZAK))
        return false;

    bot->AttackStop();
    MoveFromGroup(20.0f);
    return true;
}

bool DoomLordKazzakMoveAwayFromMarkAction::isUseful()
{
    Unit* kazzak = AI_VALUE2(Unit*, "find target", "doom lord kazzak");
    return kazzak && kazzak->IsAlive() && kazzak->IsInCombat() &&
           bot->HasAura(SPELL_DOOM_LORD_KAZZAK_MARK_OF_KAZZAK);
}

bool DoomLordKazzakMoveAwayDuringTwistedReflectionAction::Execute(Event /*event*/)
{
    Unit* kazzak = AI_VALUE2(Unit*, "find target", "doom lord kazzak");
    if (!kazzak || !kazzak->IsAlive() || !kazzak->IsInCombat() ||
        !bot->HasAura(SPELL_DOOM_LORD_KAZZAK_TWISTED_REFLECTION))
        return false;

    bot->AttackStop();

    if (botAI->IsHeal(bot))
        return true;

    MoveFromGroup(15.0f);
    return true;
}

bool DoomLordKazzakMoveAwayDuringTwistedReflectionAction::isUseful()
{
    Unit* kazzak = AI_VALUE2(Unit*, "find target", "doom lord kazzak");
    return kazzak && kazzak->IsAlive() && kazzak->IsInCombat() &&
           bot->HasAura(SPELL_DOOM_LORD_KAZZAK_TWISTED_REFLECTION);
}

bool DoomwalkerChainLightningTrigger::IsActive()
{
    if (botAI->IsTank(bot))
        return false;

    Unit* doomwalker = AI_VALUE2(Unit*, "find target", "doomwalker");
    return IsDoomwalkerCasting(doomwalker, SPELL_DOOMWALKER_CHAIN_LIGHTNING) &&
           HasGroupMemberWithinDistance(bot, DOOMWALKER_SPREAD_DISTANCE);
}

bool DoomwalkerEarthquakeTrigger::IsActive()
{
    if (botAI->IsTank(bot))
        return false;

    Unit* doomwalker = AI_VALUE2(Unit*, "find target", "doomwalker");
    if (!doomwalker || !doomwalker->IsAlive() || !doomwalker->IsInCombat() ||
        !doomwalker->HasUnitState(UNIT_STATE_CASTING) ||
        !doomwalker->FindCurrentSpellBySpellId(SPELL_DOOMWALKER_EARTHQUAKE))
        return false;

    return bot->GetExactDist2d(doomwalker) < 25.0f;
}

bool DoomLordKazzakMarkOfKazzakTrigger::IsActive()
{
    Unit* kazzak = AI_VALUE2(Unit*, "find target", "doom lord kazzak");
    return kazzak && kazzak->IsAlive() && kazzak->IsInCombat() &&
           bot->HasAura(SPELL_DOOM_LORD_KAZZAK_MARK_OF_KAZZAK);
}

bool DoomLordKazzakTwistedReflectionTrigger::IsActive()
{
    Unit* kazzak = AI_VALUE2(Unit*, "find target", "doom lord kazzak");
    return kazzak && kazzak->IsAlive() && kazzak->IsInCombat() &&
           bot->HasAura(SPELL_DOOM_LORD_KAZZAK_TWISTED_REFLECTION);
}

float DoomLordKazzakMarkOfKazzakMultiplier::GetValue(Action* action)
{
    Unit* kazzak = AI_VALUE2(Unit*, "find target", "doom lord kazzak");
    if (!kazzak || !kazzak->IsAlive() || !kazzak->IsInCombat() ||
        !bot->HasAura(SPELL_DOOM_LORD_KAZZAK_MARK_OF_KAZZAK))
        return 1.0f;

    if (IsOffensiveSpellAction(action))
        return 0.0f;

    return 1.0f;
}

float DoomLordKazzakTwistedReflectionMultiplier::GetValue(Action* action)
{
    Unit* kazzak = AI_VALUE2(Unit*, "find target", "doom lord kazzak");
    if (!kazzak || !kazzak->IsAlive() || !kazzak->IsInCombat() ||
        !bot->HasAura(SPELL_DOOM_LORD_KAZZAK_TWISTED_REFLECTION))
        return 1.0f;

    if (dynamic_cast<AttackAction*>(action) || dynamic_cast<CastShootAction*>(action) ||
        IsOffensiveSpellAction(action))
        return 0.0f;

    return 1.0f;
}

void RaidDoomwalkerStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("doomwalker chain lightning", {
        NextAction("doomwalker spread for chain lightning", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("doomwalker earthquake", {
        NextAction("doomwalker move away from earthquake", ACTION_EMERGENCY + 7) }));

    triggers.push_back(new TriggerNode("doomwalker nature resistance", {
        NextAction("doomwalker nature resistance", ACTION_RAID) }));
}

void RaidDoomLordKazzakStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("doom lord kazzak mark of kazzak", {
        NextAction("doom lord kazzak move away from mark of kazzak", ACTION_EMERGENCY + 6) }));

    triggers.push_back(new TriggerNode("doom lord kazzak twisted reflection", {
        NextAction("doom lord kazzak move away during twisted reflection", ACTION_EMERGENCY + 7) }));

    triggers.push_back(new TriggerNode("doom lord kazzak shadow resistance", {
        NextAction("doom lord kazzak shadow resistance", ACTION_RAID) }));
}

void RaidDoomLordKazzakStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new DoomLordKazzakMarkOfKazzakMultiplier(botAI));
    multipliers.push_back(new DoomLordKazzakTwistedReflectionMultiplier(botAI));
}
