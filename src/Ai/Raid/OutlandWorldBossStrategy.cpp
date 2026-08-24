/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "OutlandWorldBossStrategy.h"

#include "AttackAction.h"
#include "Group.h"
#include "GenericSpellActions.h"
#include "Playerbots.h"
#include "ReachTargetActions.h"
#include "Unit.h"

namespace
{
// Chain Lightning has a 10-yard jump radius and can hit up to ten targets.
// Leave enough margin for movement interpolation and player collision sizes.
constexpr float DOOMWALKER_SPREAD_DISTANCE = 12.0f;
constexpr float DOOMWALKER_POSITION_TOLERANCE = 1.5f;
constexpr float DOOMWALKER_MELEE_RANGE_BUFFER = 2.0f;
constexpr float DOOMWALKER_PRIMARY_RING_RADIUS = 32.0f;
constexpr float DOOMWALKER_SECONDARY_RING_RADIUS = 19.0f;
constexpr float DOOMWALKER_OVERFLOW_RING_RADIUS = 45.0f;
constexpr uint8 DOOMWALKER_PRIMARY_RING_SLOTS = 16;
constexpr uint8 DOOMWALKER_SECONDARY_RING_SLOTS = 9;

bool FindDoomwalkerSpreadPosition(PlayerbotAI* botAI, Player* bot,
                                  Unit* doomwalker, Position& destination)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    bool const isMelee = botAI->IsMelee(bot);
    uint32 botIndex = 0;
    uint32 memberCount = 0;
    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* member = gref->GetSource();
        if (!member || !member->IsAlive() || member->GetMapId() != bot->GetMapId() ||
            member == doomwalker->GetVictim() || botAI->IsMelee(member) != isMelee)
            continue;

        if (member == bot)
            botIndex = memberCount;

        ++memberCount;
    }

    if (!memberCount)
        return false;

    float radius = 0.0f;
    float angle = 0.0f;
    if (isMelee)
    {
        // Give every melee a unique position and reserve slot zero for the current
        // tank. Stay inside maximum melee range so small movement discrepancies do
        // not make normal attack positioning pull the bot back toward the boss.
        radius = std::max(1.0f, bot->GetMeleeRange(doomwalker) - DOOMWALKER_MELEE_RANGE_BUFFER);
        uint32 const slotCount = memberCount + 1;
        float const tankAngle = doomwalker->GetVictim() ?
            doomwalker->GetAngle(doomwalker->GetVictim()) : doomwalker->GetOrientation();
        angle = tankAngle + 2.0f * M_PI * static_cast<float>(botIndex + 1) /
                              static_cast<float>(slotCount);
    }
    else
    {
        // A normal 25-player composition fits on the 32-yard ring at useful
        // casting range. Ring capacities preserve at least 12 yards between
        // adjacent slots; extra ranged use rings 13 yards apart.
        uint32 ringIndex = botIndex;
        uint32 ringCount = std::min<uint32>(memberCount, DOOMWALKER_PRIMARY_RING_SLOTS);
        radius = DOOMWALKER_PRIMARY_RING_RADIUS;

        if (botIndex >= DOOMWALKER_PRIMARY_RING_SLOTS)
        {
            ringIndex -= DOOMWALKER_PRIMARY_RING_SLOTS;
            ringCount = std::min<uint32>(
                memberCount - DOOMWALKER_PRIMARY_RING_SLOTS,
                DOOMWALKER_SECONDARY_RING_SLOTS);
            radius = DOOMWALKER_SECONDARY_RING_RADIUS;
        }

        if (botIndex >= DOOMWALKER_PRIMARY_RING_SLOTS + DOOMWALKER_SECONDARY_RING_SLOTS)
        {
            ringIndex -= DOOMWALKER_SECONDARY_RING_SLOTS;
            ringCount = memberCount - DOOMWALKER_PRIMARY_RING_SLOTS -
                        DOOMWALKER_SECONDARY_RING_SLOTS;
            radius = DOOMWALKER_OVERFLOW_RING_RADIUS;
        }

        angle = 2.0f * M_PI * static_cast<float>(ringIndex) /
                static_cast<float>(ringCount);
        if (radius == DOOMWALKER_SECONDARY_RING_RADIUS)
            angle += M_PI / static_cast<float>(ringCount);
    }

    float x = doomwalker->GetPositionX() + radius * std::cos(angle);
    float y = doomwalker->GetPositionY() + radius * std::sin(angle);
    float z = bot->GetPositionZ();
    if (!bot->GetMap()->CheckCollisionAndGetValidCoords(
            bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
            x, y, z))
    {
        return false;
    }

    destination.Relocate(x, y, z);
    return true;
}

bool NeedsDoomwalkerSpread(PlayerbotAI* botAI, Player* bot, Unit* doomwalker)
{
    if (!doomwalker || !doomwalker->IsAlive() || !doomwalker->IsInCombat() ||
        doomwalker->GetVictim() == bot)
        return false;

    Position destination;
    return FindDoomwalkerSpreadPosition(botAI, bot, doomwalker, destination) &&
           bot->GetExactDist2d(destination.GetPositionX(), destination.GetPositionY()) >
               DOOMWALKER_POSITION_TOLERANCE;
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
    Unit* doomwalker = AI_VALUE2(Unit*, "find target", "doomwalker");
    if (!NeedsDoomwalkerSpread(botAI, bot, doomwalker))
        return false;

    Position spreadPosition;
    if (!FindDoomwalkerSpreadPosition(botAI, bot, doomwalker, spreadPosition))
        return false;

    return MoveTo(bot->GetMapId(), spreadPosition.GetPositionX(),
                  spreadPosition.GetPositionY(), spreadPosition.GetPositionZ(),
                  false, false, false, false,
                  MovementPriority::MOVEMENT_COMBAT, true);
}

bool DoomwalkerChainLightningSpreadAction::isUseful()
{
    Unit* doomwalker = AI_VALUE2(Unit*, "find target", "doomwalker");
    return NeedsDoomwalkerSpread(botAI, bot, doomwalker);
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
    Unit* doomwalker = AI_VALUE2(Unit*, "find target", "doomwalker");
    return NeedsDoomwalkerSpread(botAI, bot, doomwalker);
}

float DoomwalkerMeleePositionMultiplier::GetValue(Action* action)
{
    Unit* doomwalker = AI_VALUE2(Unit*, "find target", "doomwalker");
    if (!doomwalker || !doomwalker->IsAlive() || !doomwalker->IsInCombat() ||
        doomwalker->GetVictim() == bot || !botAI->IsMelee(bot))
        return 1.0f;

    if (dynamic_cast<CombatFormationMoveAction*>(action) || dynamic_cast<ReachMeleeAction*>(action))
        return 0.0f;

    return 1.0f;
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

    triggers.push_back(new TriggerNode("doomwalker nature resistance", {
        NextAction("doomwalker nature resistance", ACTION_RAID) }));
}

void RaidDoomwalkerStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new DoomwalkerMeleePositionMultiplier(botAI));
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
