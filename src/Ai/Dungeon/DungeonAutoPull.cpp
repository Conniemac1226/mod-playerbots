#include "DungeonAutoPull.h"

#include "AttackersValue.h"
#include "Group.h"
#include "Playerbots.h"
#include "Value.h"
#include <cmath>
#include <limits>
#include <vector>

namespace
{
constexpr float AUTO_PULL_SEARCH_RANGE = 55.0f;
constexpr float AUTO_PULL_GROUP_RANGE = 35.0f;
constexpr float AUTO_PULL_HEALER_MANA_PCT = 45.0f;
constexpr float AUTO_PULL_MEMBER_HP_PCT = 60.0f;
constexpr float AUTO_PULL_TANK_HP_PCT = 75.0f;
constexpr float AUTO_PULL_FORWARD_ARC = static_cast<float>(M_PI) * 0.75f;
constexpr float AUTO_PULL_IDEAL_MIN_RANGE = 18.0f;
constexpr float AUTO_PULL_IDEAL_MAX_RANGE = 38.0f;
constexpr float AUTO_PULL_TOO_CLOSE_RANGE = 12.0f;
constexpr float AUTO_PULL_VERTICAL_PENALTY_STEP = 2.5f;
constexpr float AUTO_PULL_PACK_RADIUS = 12.0f;
constexpr float AUTO_PULL_SIDE_PACK_RADIUS = 20.0f;
constexpr float AUTO_PULL_BOSS_TRASH_GUARD_RADIUS = 30.0f;

struct PullCandidateScore
{
    Unit* unit = nullptr;
    float score = -std::numeric_limits<float>::max();
};

float GetFacingDelta(Player* bot, Unit* unit)
{
    float delta = Position::NormalizeOrientation(bot->GetAngle(unit) - bot->GetOrientation());
    return std::min(delta, static_cast<float>((2.0 * M_PI) - delta));
}

bool IsBossUnit(Unit* unit)
{
    Creature* creature = unit ? unit->ToCreature() : nullptr;
    return creature && (creature->IsDungeonBoss() || creature->isWorldBoss());
}

float ScorePullDistance(float distance)
{
    if (distance > AUTO_PULL_SEARCH_RANGE)
        return -1000.0f;

    if (distance < AUTO_PULL_TOO_CLOSE_RANGE)
        return -55.0f - (AUTO_PULL_TOO_CLOSE_RANGE - distance) * 3.0f;

    if (distance < AUTO_PULL_IDEAL_MIN_RANGE)
        return 8.0f + (distance - AUTO_PULL_TOO_CLOSE_RANGE) * 2.0f;

    if (distance <= AUTO_PULL_IDEAL_MAX_RANGE)
    {
        float const mid = (AUTO_PULL_IDEAL_MIN_RANGE + AUTO_PULL_IDEAL_MAX_RANGE) * 0.5f;
        return 34.0f - std::abs(distance - mid) * 1.15f;
    }

    return 14.0f - (distance - AUTO_PULL_IDEAL_MAX_RANGE) * 1.35f;
}

std::vector<Unit*> CollectAdvancePullCandidates(PlayerbotAI* botAI, Player* bot)
{
    std::vector<Unit*> candidates;
    if (!botAI || !bot)
        return candidates;

    auto collect = [&](std::string const& valueName)
    {
        if (Value<GuidVector>* units = botAI->GetAiObjectContext()->GetValue<GuidVector>(valueName))
        {
            for (ObjectGuid const& guid : units->Get())
            {
                Unit* unit = botAI->GetUnit(guid);
                if (!unit || !unit->IsAlive() || unit->IsInCombat())
                    continue;

                if (!AttackersValue::IsPossibleTarget(unit, bot))
                    continue;

                if (!bot->IsWithinLOSInMap(unit))
                    continue;

                if (bot->GetDistance(unit) > AUTO_PULL_SEARCH_RANGE)
                    continue;

                candidates.push_back(unit);
            }
        }
    };

    collect("nearest hostile npcs");

    if (candidates.empty())
        collect("nearest npcs");

    return candidates;
}

PullCandidateScore ScoreAdvancePullTarget(Player* bot, Unit* candidate, std::vector<Unit*> const& candidates)
{
    PullCandidateScore result;
    if (!bot || !candidate)
        return result;

    result.unit = candidate;

    float const distance3d = bot->GetDistance(candidate);
    float const distance2d = bot->GetExactDist2d(candidate);
    float const heightDelta = std::abs(bot->GetPositionZ() - candidate->GetPositionZ());
    float const facingDelta = GetFacingDelta(bot, candidate);
    bool const isBoss = IsBossUnit(candidate);

    float score = 0.0f;

    score += ScorePullDistance(distance3d);

    float const forwardRatio = 1.0f - std::min(facingDelta, static_cast<float>(M_PI)) / static_cast<float>(M_PI);
    score += forwardRatio * 28.0f;
    if (bot->HasInArc(AUTO_PULL_FORWARD_ARC, candidate))
        score += 10.0f;
    else
        score -= 12.0f;

    score -= std::min(heightDelta, 15.0f) * AUTO_PULL_VERTICAL_PENALTY_STEP;

    if (distance3d - distance2d > 6.0f)
        score -= (distance3d - distance2d) * 2.0f;

    uint32 tightNeighbors = 0;
    uint32 sidePackNeighbors = 0;
    uint32 verticalSplitNeighbors = 0;
    uint32 nearbyTrashAroundBoss = 0;

    float const candidateAngle = bot->GetAngle(candidate);

    for (Unit* other : candidates)
    {
        if (!other || other == candidate)
            continue;

        float const separation2d = candidate->GetExactDist2d(other);
        float const relativeHeight = std::abs(candidate->GetPositionZ() - other->GetPositionZ());

        if (separation2d <= AUTO_PULL_PACK_RADIUS && relativeHeight <= 4.0f)
            ++tightNeighbors;

        if (isBoss && !IsBossUnit(other) && separation2d <= AUTO_PULL_BOSS_TRASH_GUARD_RADIUS && relativeHeight <= 6.0f)
            ++nearbyTrashAroundBoss;

        if (separation2d <= AUTO_PULL_SIDE_PACK_RADIUS)
        {
            float angleSeparation =
                Position::NormalizeOrientation(candidateAngle - bot->GetAngle(other));
            angleSeparation = std::min(angleSeparation, static_cast<float>((2.0 * M_PI) - angleSeparation));

            if (separation2d > AUTO_PULL_PACK_RADIUS && angleSeparation > static_cast<float>(M_PI) / 5.0f)
                ++sidePackNeighbors;

            if (relativeHeight > 6.0f)
                ++verticalSplitNeighbors;
        }
    }

    score += std::min<uint32>(tightNeighbors, 4) * 6.0f;
    score -= std::min<uint32>(sidePackNeighbors, 5) * 7.0f;
    score -= std::min<uint32>(verticalSplitNeighbors, 4) * 5.0f;

    if (isBoss)
    {
        score -= 40.0f;

        if (nearbyTrashAroundBoss > 0)
            score -= 120.0f + std::min<uint32>(nearbyTrashAroundBoss, 4) * 15.0f;
    }

    result.score = score;
    return result;
}

bool IsGroupReadyForAdvancePull(PlayerbotAI* botAI, Player* bot)
{
    if (!botAI || !bot || !bot->GetMap() || !bot->GetMap()->IsDungeon())
        return false;

    if (!bot->GetGroup() || !bot->IsAlive() || bot->IsInCombat() || !botAI->IsMainTank(bot))
        return false;

    if (botAI->HasStrategy("stay", BOT_STATE_NON_COMBAT) || botAI->HasStrategy("passive", BOT_STATE_NON_COMBAT))
        return false;

    AiObjectContext* context = botAI->GetAiObjectContext();
    if (!context)
        return false;

    if (context->GetValue<uint8>("attacker count")->Get() != 0)
        return false;

    if (Unit* currentTarget = context->GetValue<Unit*>("current target")->Get())
    {
        if (currentTarget->IsAlive() && currentTarget->IsInWorld() && currentTarget->GetMapId() == bot->GetMapId())
            return false;
    }

    if (bot->HealthBelowPct(AUTO_PULL_TANK_HP_PCT))
        return false;

    bool foundHealer = false;
    bool healerReady = false;

    Group* group = bot->GetGroup();
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot)
            continue;

        if (!member->IsAlive() || member->IsBeingTeleported() || member->IsInCombat())
            return false;

        if (bot->GetMapId() != member->GetMapId() || bot->GetDistance(member) > AUTO_PULL_GROUP_RANGE)
            return false;

        if (member->HealthBelowPct(AUTO_PULL_MEMBER_HP_PCT))
            return false;

        if (!foundHealer && botAI->IsHeal(member))
        {
            foundHealer = true;
            healerReady = member->getPowerType() != POWER_MANA ||
                member->GetPowerPct(POWER_MANA) >= AUTO_PULL_HEALER_MANA_PCT;
        }
    }

    return !foundHealer || healerReady;
}

Unit* SelectAdvancePullTarget(PlayerbotAI* botAI, Player* bot)
{
    if (!botAI || !bot)
        return nullptr;

    std::vector<Unit*> candidates = CollectAdvancePullCandidates(botAI, bot);
    PullCandidateScore bestCandidate;

    for (Unit* candidate : candidates)
    {
        PullCandidateScore scored = ScoreAdvancePullTarget(bot, candidate, candidates);
        if (scored.score > bestCandidate.score)
            bestCandidate = scored;
    }

    return bestCandidate.score > 0.0f ? bestCandidate.unit : nullptr;
}

bool TryTankRangedOpener(PlayerbotAI* botAI, Player* bot, Event& event)
{
    if (!botAI || !bot || !botAI->IsTank(bot))
        return false;

    switch (bot->getClass())
    {
        case CLASS_PALADIN:
            return botAI->DoSpecificAction("avenger's shield", event, true) ||
                   botAI->DoSpecificAction("hand of reckoning", event, true);
        case CLASS_WARRIOR:
            return botAI->DoSpecificAction("heroic throw", event, true);
        case CLASS_DRUID:
            return botAI->DoSpecificAction("faerie fire (feral)", event, true) ||
                   botAI->DoSpecificAction("growl", event, true);
        case CLASS_DEATH_KNIGHT:
            return botAI->DoSpecificAction("death grip", event, true) ||
                   botAI->DoSpecificAction("icy touch", event, true) ||
                   botAI->DoSpecificAction("dark command", event, true);
        default:
            return false;
    }
}
}

void DungeonAutoPull::AddDefaultPullTrigger(std::vector<TriggerNode*>& triggers)
{
    AddDefaultPullTrigger(triggers, ACTION_MOVE + 3.0f);
}

void DungeonAutoPull::AddDefaultPullTrigger(std::vector<TriggerNode*>& triggers, float relevance)
{
    triggers.push_back(new TriggerNode("dungeon auto pull ready", { NextAction("dungeon auto pull", relevance) }));
}

bool DungeonAutoPullReadyTrigger::IsActive()
{
    Player* bot = botAI->GetBot();
    return IsGroupReadyForAdvancePull(botAI, bot) && SelectAdvancePullTarget(botAI, bot);
}

bool DungeonAutoPullAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    if (!IsGroupReadyForAdvancePull(botAI, bot))
        return false;

    Unit* target = SelectAdvancePullTarget(botAI, bot);
    if (!target)
        return false;

    context->GetValue<Unit*>("current target")->Set(target);
    bot->SetSelection(target->GetGUID());

    bool const usedRangedOpener = TryTankRangedOpener(botAI, bot, event);
    bool const attackStarted = Attack(target);
    if (usedRangedOpener || attackStarted)
    {
        botAI->SetNextCheckDelay(sPlayerbotAIConfig.reactDelay);
        return true;
    }

    return false;
}

bool DungeonAutoPullAction::isUseful()
{
    Player* bot = botAI->GetBot();
    return IsGroupReadyForAdvancePull(botAI, bot) && SelectAdvancePullTarget(botAI, bot);
}
