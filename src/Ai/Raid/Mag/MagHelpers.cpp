#include "MagHelpers.h"
#include "AttackersValue.h"
#include "Creature.h"
#include "GameObject.h"
#include "Log.h"
#include "Map.h"
#include "ObjectGuid.h"
#include "Playerbots.h"
#include "Bot/Factory/AiFactory.h"
#include "Value.h"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>

namespace MagtheridonHelpers
{
    namespace
    {
        std::unordered_map<std::string, time_t> magtheridonDebugThrottleUntil;
        std::unordered_map<ObjectGuid, time_t> magtheridonLastActivityAt;
        std::unordered_map<ObjectGuid, std::string> magtheridonLastActivity;

        constexpr uint32 MAGTHERIDON_DEBUG_MIN_THROTTLE_SECONDS = 5;
        constexpr float MAGTHERIDON_FALLBACK_SEARCH_RADIUS = 90.0f;
        constexpr float MAGTHERIDON_ENCOUNTER_AREA_RADIUS = 60.0f;
        constexpr float AUTO_PULL_SEARCH_RANGE = 55.0f;
        constexpr float AUTO_PULL_GROUP_RANGE = 35.0f;
        constexpr float AUTO_PULL_HEALER_MANA_PCT = 45.0f;
        constexpr float AUTO_PULL_MEMBER_HP_PCT = 60.0f;
        constexpr float AUTO_PULL_TANK_HP_PCT = 75.0f;
        constexpr float TRASH_PHASE_SEARCH_RADIUS = 120.0f;
        constexpr float AUTO_PULL_FORWARD_ARC = static_cast<float>(M_PI) * 0.75f;
        constexpr float AUTO_PULL_IDEAL_MIN_RANGE = 18.0f;
        constexpr float AUTO_PULL_IDEAL_MAX_RANGE = 38.0f;
        constexpr float AUTO_PULL_TOO_CLOSE_RANGE = 12.0f;
        constexpr float AUTO_PULL_VERTICAL_PENALTY_STEP = 2.5f;
        constexpr float AUTO_PULL_PACK_RADIUS = 12.0f;
        constexpr float AUTO_PULL_SIDE_PACK_RADIUS = 20.0f;

        struct PullCandidateScore
        {
            Unit* unit = nullptr;
            float score = -std::numeric_limits<float>::max();
        };

        float GetFacingDelta(Player* bot, Unit* unit)
        {
            float delta = Position::NormalizeOrientation(
                bot->GetAngle(unit) - bot->GetOrientation());
            return std::min(delta, static_cast<float>((2.0 * M_PI) - delta));
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
                float const mid =
                    (AUTO_PULL_IDEAL_MIN_RANGE + AUTO_PULL_IDEAL_MAX_RANGE) * 0.5f;
                return 34.0f - std::abs(distance - mid) * 1.15f;
            }

            return 14.0f - (distance - AUTO_PULL_IDEAL_MAX_RANGE) * 1.35f;
        }

        bool IsInsideMagtheridonFightArea(Position const& pos)
        {
            return pos.GetPositionZ() >= -8.0f && pos.GetPositionZ() <= 12.0f &&
                   Position(1.359f, 2.048f, -0.406f).GetExactDist2d(pos.GetPositionX(), pos.GetPositionY()) <= MAGTHERIDON_ENCOUNTER_AREA_RADIUS;
        }

        bool IsMagtheridonEncounterUnit(Unit* unit)
        {
            if (!unit || !unit->IsAlive())
                return false;

            switch (unit->GetEntry())
            {
                case NPC_MAGTHERIDON:
                case NPC_HELLFIRE_CHANNELER:
                case NPC_BURNING_ABYSSAL:
                case NPC_TARGET_TRIGGER:
                    return true;
                default:
                    return false;
            }
        }

        bool IsMagtheridonEncounterRelevant(PlayerbotAI* botAI, Player* bot, Unit* magtheridon)
        {
            if (!botAI || !bot || bot->GetMapId() != MAGTHERIDON_MAP_ID)
                return false;

            Position botPos(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
            if (IsInsideMagtheridonFightArea(botPos))
                return true;

            if (!magtheridon)
                magtheridon = FindMagtheridon(botAI, bot);

            if (IsMagtheridonEncounterUnit(magtheridon))
                return true;

            if (magtheridon && magtheridon->GetDistance(bot) <= 80.0f)
                return true;

            for (uint32 channelerGuid : { SOUTH_CHANNELER, WEST_CHANNELER, NORTHWEST_CHANNELER, EAST_CHANNELER, NORTHEAST_CHANNELER })
            {
                if (Creature* channeler = GetChanneler(bot, channelerGuid))
                {
                    if (channeler->IsInCombat() || bot->GetDistance(channeler) <= 60.0f)
                        return true;
                }
            }

            Unit* currentTarget = botAI->GetUnit(bot->GetTarget());
            if (IsMagtheridonEncounterUnit(currentTarget))
                return true;

            return false;
        }

        std::string GetMagtheridonMovementDebug(PlayerbotAI* botAI, Player* bot, Unit* magtheridon)
        {
            std::ostringstream out;
            if (!bot)
                return "movement=none";

            out << "movement=stopped=" << (bot->IsStopped() ? 1 : 0)
                << " follow_nc=" << (botAI && botAI->HasStrategy("follow", BotState::BOT_STATE_NON_COMBAT) ? 1 : 0)
                << " formation_c=" << (botAI && botAI->HasStrategy("formation", BotState::BOT_STATE_COMBAT) ? 1 : 0)
                << " stay_c=" << (botAI && botAI->HasStrategy("stay", BotState::BOT_STATE_COMBAT) ? 1 : 0)
                << " stay_nc=" << (botAI && botAI->HasStrategy("stay", BotState::BOT_STATE_NON_COMBAT) ? 1 : 0)
                << " move_from_group_c=" << (botAI && botAI->HasStrategy("move from group", BotState::BOT_STATE_COMBAT) ? 1 : 0)
                << " move_from_group_nc=" << (botAI && botAI->HasStrategy("move from group", BotState::BOT_STATE_NON_COMBAT) ? 1 : 0);

            if (magtheridon && bot->GetMapId() == MAGTHERIDON_MAP_ID)
            {
                bool inFront = magtheridon->HasInArc(static_cast<float>(M_PI) / 2.0f, bot);
                Unit* mainTank = botAI && botAI->GetAiObjectContext()
                    ? botAI->GetAiObjectContext()->GetValue<Unit*>("main tank")->Get()
                    : nullptr;

                out << std::fixed << std::setprecision(2)
                    << " boss_dist=" << bot->GetDistance(magtheridon)
                    << " boss_dist2d=" << bot->GetExactDist2d(magtheridon)
                    << " boss_front_arc=" << (inFront ? 1 : 0)
                    << " bot_angle_to_boss=" << bot->GetAngle(magtheridon)
                    << " boss_angle_to_bot=" << magtheridon->GetAngle(bot);

                if (mainTank && mainTank != bot)
                    out << " tank_dist=" << bot->GetDistance(mainTank)
                        << " tank_dist2d=" << bot->GetExactDist2d(mainTank);
            }

            return out.str();
        }
    }

    std::string GetMagtheridonDebugRole(PlayerbotAI* botAI, Player* bot)
    {
        if (!botAI || !bot)
            return "role=unknown";

        std::ostringstream out;
        out << "role=";
        if (botAI->IsMainTank(bot))
            out << "main_tank";
        else if (botAI->IsAssistTankOfIndex(bot, 0))
            out << "assist_tank_0";
        else if (botAI->IsAssistTankOfIndex(bot, 1))
            out << "assist_tank_1";
        else if (botAI->IsHeal(bot))
            out << "healer";
        else if (botAI->IsRanged(bot))
            out << "ranged";
        else if (botAI->IsTank(bot))
            out << "tank";
        else
            out << "melee";

        out << " class=" << static_cast<uint32>(bot->getClass())
            << " spec=" << AiFactory::GetPlayerSpecName(bot);
        return out.str();
    }

    std::string GetMagtheridonBotRole(PlayerbotAI* botAI, Player* bot)
    {
        if (!botAI || !bot)
            return "unknown";

        if (botAI->IsMainTank(bot))
            return "main_tank";
        if (botAI->IsAssistTankOfIndex(bot, 0))
            return "assist_tank_0";
        if (botAI->IsAssistTankOfIndex(bot, 1))
            return "assist_tank_1";
        if (botAI->IsHeal(bot))
            return "healer";
        if (botAI->IsRanged(bot))
            return "ranged";
        if (botAI->IsTank(bot))
            return "tank";
        return "melee";
    }

    std::string GetMagtheridonDebugUnit(Player* bot, Unit* unit)
    {
        if (!unit)
            return "target=none";

        std::ostringstream out;
        out << "target_name=" << unit->GetName()
            << " target_entry=" << unit->GetEntry()
            << " target_guid=" << unit->GetGUID().ToString();

        if (bot)
        {
            out << " target_dist=" << std::fixed << std::setprecision(2) << bot->GetDistance(unit)
                << " target_hp=" << unit->GetHealth()
                << " target_hp_pct=" << std::fixed << std::setprecision(1) << unit->GetHealthPct();
        }

        return out.str();
    }

    std::string GetMagtheridonTargetIdentityFields(std::string const& prefix, Unit* unit)
    {
        std::ostringstream out;
        out << prefix << "_name=";
        if (unit)
            out << unit->GetName();
        else
            out << "none";

        out << " " << prefix << "_entry=" << (unit ? unit->GetEntry() : 0)
            << " " << prefix << "_guid=";
        if (unit)
            out << unit->GetGUID().ToString();
        else
            out << "none";

        return out.str();
    }

    std::string GetMagtheridonTargetDecisionFields(Unit* previousTarget, Unit* selectedTarget, Unit* rejectedTarget, std::string const& actionThatWon, std::string const& actionThatLostOrWasSuppressed)
    {
        std::ostringstream out;
        out << GetMagtheridonTargetIdentityFields("selected_target", selectedTarget)
            << " " << GetMagtheridonTargetIdentityFields("rejected_candidate", rejectedTarget)
            << " " << GetMagtheridonTargetIdentityFields("previous_target", previousTarget)
            << " action_that_won=" << actionThatWon
            << " action_that_lost_or_was_suppressed=" << actionThatLostOrWasSuppressed;
        return out.str();
    }

    std::string GetMagtheridonDebugActivity(Player* bot)
    {
        if (!bot)
            return "last_activity=none";

        auto it = magtheridonLastActivity.find(bot->GetGUID());
        auto tsIt = magtheridonLastActivityAt.find(bot->GetGUID());
        if (it == magtheridonLastActivity.end() || tsIt == magtheridonLastActivityAt.end())
            return "last_activity=none";

        std::ostringstream out;
        out << "last_activity=" << it->second << " last_activity_age=" << (time(nullptr) - tsIt->second);
        return out.str();
    }

    void MarkMagtheridonDebugActivity(Player* bot, std::string const& event)
    {
        if (!bot)
            return;

        magtheridonLastActivityAt[bot->GetGUID()] = time(nullptr);
        magtheridonLastActivity[bot->GetGUID()] = event;
    }

    bool ShouldLogMagtheridonDebug(Player* bot, std::string const& key, uint32 throttleSeconds)
    {
        if (!bot)
            return false;

        throttleSeconds = std::max<uint32>(throttleSeconds, MAGTHERIDON_DEBUG_MIN_THROTTLE_SECONDS);

        const time_t now = time(nullptr);
        const std::string throttleKey = bot->GetGUID().ToString() + ":" + key;
        auto it = magtheridonDebugThrottleUntil.find(throttleKey);
        if (it != magtheridonDebugThrottleUntil.end() && now < it->second)
            return false;

        magtheridonDebugThrottleUntil[throttleKey] = now + throttleSeconds;
        return true;
    }

    std::string GetMagtheridonEncounterDebug(PlayerbotAI* botAI, Player* bot, Unit* magtheridon)
    {
        std::ostringstream out;
        out << "map=" << (bot ? bot->GetMapId() : 0);

        if (!magtheridon && botAI && bot)
            magtheridon = FindMagtheridon(botAI, bot);

        bool bossAlive = magtheridon && magtheridon->IsAlive();
        bool shadowCage = bossAlive && magtheridon->HasAura(SPELL_SHADOW_CAGE);
        bool blastNova = bossAlive && magtheridon->HasUnitState(UNIT_STATE_CASTING) &&
                         magtheridon->FindCurrentSpellBySpellId(SPELL_BLAST_NOVA);
        bool encounterRelevant = IsMagtheridonEncounterRelevant(botAI, bot, magtheridon);

        uint32 channelersAlive = 0;
        for (uint32 channelerGuid : { SOUTH_CHANNELER, WEST_CHANNELER, NORTHWEST_CHANNELER, EAST_CHANNELER, NORTHEAST_CHANNELER })
        {
            if (Creature* channeler = bot ? GetChanneler(bot, channelerGuid) : nullptr)
                ++channelersAlive;
        }

        uint32 abyssalsAlive = 0;
        uint32 cubesAssigned = 0;
        if (botAI)
        {
            if (Value<GuidVector>* npcsValue = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest hostile npcs"))
            {
                const GuidVector& npcs = npcsValue->Get();
                for (ObjectGuid const& npcGuid : npcs)
                {
                    Unit* unit = botAI->GetUnit(npcGuid);
                    if (unit && unit->IsAlive() && unit->GetEntry() == NPC_BURNING_ABYSSAL)
                        ++abyssalsAlive;
                }
            }
        }

        if (bot)
        {
            cubesAssigned = botToCubeAssignment.count(bot->GetGUID()) ? 1u : 0u;
        }

        out << " boss=" << (bossAlive ? "alive" : "dead")
            << " mag_alive=" << (bossAlive ? 1 : 0)
            << " boss_hp_pct=" << (bossAlive ? static_cast<int>(magtheridon->GetHealthPct()) : 0)
            << " shadow_cage=" << (shadowCage ? 1 : 0)
            << " blast_nova=" << (blastNova ? 1 : 0)
            << " channelers_alive=" << channelersAlive
            << " abyssals_alive=" << abyssalsAlive
            << " cubes_assigned=" << cubesAssigned;

        if (bot)
        {
            Unit* currentTarget = botAI ? botAI->GetUnit(bot->GetTarget()) : nullptr;
            out << " bot_in_combat=" << (bot->IsInCombat() ? 1 : 0)
                << " bot_hp_pct=" << static_cast<int>(bot->GetHealthPct())
                << " bot_mana_pct=" << static_cast<int>(bot->GetPowerPct(POWER_MANA))
                << " bot_role=" << GetMagtheridonBotRole(botAI, bot)
                << " bot_inside_room=" << (IsInsideMagtheridonFightArea(Position(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ())) ? 1 : 0)
                << " encounter_relevant=" << (encounterRelevant ? 1 : 0)
                << " current_target=";
            if (currentTarget)
                out << currentTarget->GetName() << "#" << currentTarget->GetEntry() << "#" << currentTarget->GetGUID().ToString();
            else
                out << "none";

            out << " " << GetMagtheridonMovementDebug(botAI, bot, magtheridon);
            out << " " << GetMagtheridonDebugActivity(bot);
        }

        return out.str();
    }

    void LogMagtheridonDebug(PlayerbotAI* botAI, Player* bot, std::string const& event, std::string const& details, Unit* magtheridon, uint32 throttleSeconds)
    {
        if (!bot || !IsMagtheridonEncounterRelevant(botAI, bot, magtheridon))
            return;

        if (!ShouldLogMagtheridonDebug(bot, event, throttleSeconds))
            return;

        MarkMagtheridonDebugActivity(bot, event);
        LOG_INFO("playerbots",
            "MAG-LAIR-DEBUG ts={} bot={} {} event={} {} {} {}",
            static_cast<long long>(time(nullptr)),
            bot->GetName(),
            GetMagtheridonDebugRole(botAI, bot),
            event,
            details,
            GetMagtheridonDebugUnit(bot, botAI ? botAI->GetUnit(bot->GetTarget()) : nullptr),
            GetMagtheridonEncounterDebug(botAI, bot, magtheridon));
    }

    const Position WAITING_FOR_MAGTHERIDON_POSITION = {   1.359f,   2.048f, -0.406f, 3.135f };
    const Position MAGTHERIDON_TANK_POSITION =        {  22.827f,   2.105f, -0.406f, 3.135f };
    const Position NW_CHANNELER_TANK_POSITION =       { -11.764f,  30.818f, -0.411f,   0.0f };
    const Position NE_CHANNELER_TANK_POSITION =       { -12.490f, -26.211f, -0.411f,   0.0f };
    const Position RANGED_SPREAD_POSITION =           { -14.890f,   1.995f, -0.406f,   0.0f };
    const Position HEALER_SPREAD_POSITION =           {  -2.265f,   1.874f, -0.404f,   0.0f };

    Unit* FindMagtheridon(PlayerbotAI* botAI, Player* bot)
    {
        Unit* magtheridon = botAI ? botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "magtheridon")->Get() : nullptr;
        if (magtheridon && !magtheridon->IsAlive())
            magtheridon = nullptr;
        if (magtheridon)
            return magtheridon;

        if (!botAI || !bot)
            return nullptr;

        auto scan = [&](std::string const& valueName) -> Unit*
        {
            if (Value<GuidVector>* units = botAI->GetAiObjectContext()->GetValue<GuidVector>(valueName))
            {
                for (ObjectGuid const& guid : units->Get())
                {
                    Unit* unit = botAI->GetUnit(guid);
                    if (!unit || !unit->IsAlive() ||
                        unit->GetEntry() != NPC_MAGTHERIDON)
                        continue;

                    if (unit->GetMapId() != bot->GetMapId())
                        continue;

                    if (unit->GetDistance2d(MAGTHERIDON_TANK_POSITION.GetPositionX(),
                                            MAGTHERIDON_TANK_POSITION.GetPositionY()) >
                        MAGTHERIDON_FALLBACK_SEARCH_RADIUS)
                        continue;

                    return unit;
                }
            }

            return nullptr;
        };

        magtheridon = scan("nearest hostile npcs");
        if (!magtheridon)
            magtheridon = scan("nearest npcs");

        return magtheridon;
    }

    Unit* SelectMagtheridonTrashPullTarget(PlayerbotAI* botAI, Player* bot)
    {
        if (!botAI || !bot || bot->GetMapId() != MAGTHERIDON_MAP_ID)
            return nullptr;

        std::vector<ObjectGuid> excludedGuids;
        if (Unit* magtheridon = FindMagtheridon(botAI, bot))
            excludedGuids.push_back(magtheridon->GetGUID());

        for (uint32 channelerGuid : { SOUTH_CHANNELER, WEST_CHANNELER, NORTHWEST_CHANNELER, EAST_CHANNELER, NORTHEAST_CHANNELER })
        {
            if (Creature* channeler = GetChanneler(bot, channelerGuid))
                excludedGuids.push_back(channeler->GetGUID());
        }

        std::vector<Unit*> candidates;

        auto scan = [&](std::string const& valueName)
        {
            if (Value<GuidVector>* units = botAI->GetAiObjectContext()->GetValue<GuidVector>(valueName))
            {
                for (ObjectGuid const& guid : units->Get())
                {
                    Unit* unit = botAI->GetUnit(guid);
                    std::string rejectReason;
                    if (!unit)
                        rejectReason = "unit_missing";
                    else if (!unit->IsAlive())
                        rejectReason = "dead";
                    else if (unit->IsInCombat())
                        rejectReason = "in_combat";
                    else if (unit->GetMapId() != bot->GetMapId())
                        rejectReason = "wrong_map";
                    else if (std::find(excludedGuids.begin(), excludedGuids.end(), guid) != excludedGuids.end())
                        rejectReason = "excluded";
                    else if (!AttackersValue::IsPossibleTarget(unit, bot))
                        rejectReason = "not_possible_target";
                    else if (!bot->IsWithinLOSInMap(unit))
                        rejectReason = "los";
                    else if (bot->GetDistance(unit) > AUTO_PULL_SEARCH_RANGE)
                        rejectReason = "out_of_range";

                    if (!rejectReason.empty())
                    {
                        LogMagtheridonDebug(botAI, bot, "trash_pull_candidate_reject",
                            "source=" + valueName + " " + GetMagtheridonDebugUnit(bot, unit) +
                            " reason=" + rejectReason, nullptr, 10);
                        continue;
                    }

                    candidates.push_back(unit);
                }
            }
        };

        scan("nearest hostile npcs");
        if (candidates.empty())
            scan("nearest npcs");

        PullCandidateScore bestCandidate;
        for (Unit* candidate : candidates)
        {
            if (!candidate)
                continue;

            PullCandidateScore score;
            score.unit = candidate;

            float const distance3d = bot->GetDistance(candidate);
            float const distance2d = bot->GetExactDist2d(candidate);
            float const heightDelta =
                std::abs(bot->GetPositionZ() - candidate->GetPositionZ());
            float const facingDelta = GetFacingDelta(bot, candidate);

            float value = 0.0f;
            value += ScorePullDistance(distance3d);

            float const forwardRatio =
                1.0f - std::min(facingDelta, static_cast<float>(M_PI)) /
                static_cast<float>(M_PI);
            value += forwardRatio * 28.0f;
            if (bot->HasInArc(AUTO_PULL_FORWARD_ARC, candidate))
                value += 10.0f;
            else
                value -= 12.0f;

            value -= std::min(heightDelta, 15.0f) * AUTO_PULL_VERTICAL_PENALTY_STEP;

            if (distance3d - distance2d > 6.0f)
                value -= (distance3d - distance2d) * 2.0f;

            uint32 tightNeighbors = 0;
            uint32 sidePackNeighbors = 0;
            uint32 verticalSplitNeighbors = 0;
            float const candidateAngle = bot->GetAngle(candidate);

            for (Unit* other : candidates)
            {
                if (!other || other == candidate)
                    continue;

                float const separation2d = candidate->GetExactDist2d(other);
                float const relativeHeight =
                    std::abs(candidate->GetPositionZ() - other->GetPositionZ());

                if (separation2d <= AUTO_PULL_PACK_RADIUS && relativeHeight <= 4.0f)
                    ++tightNeighbors;

                if (separation2d <= AUTO_PULL_SIDE_PACK_RADIUS)
                {
                    float angleSeparation =
                        Position::NormalizeOrientation(candidateAngle - bot->GetAngle(other));
                    angleSeparation = std::min(
                        angleSeparation,
                        static_cast<float>((2.0 * M_PI) - angleSeparation));

                    if (separation2d > AUTO_PULL_PACK_RADIUS &&
                        angleSeparation > static_cast<float>(M_PI) / 5.0f)
                        ++sidePackNeighbors;

                    if (relativeHeight > 6.0f)
                        ++verticalSplitNeighbors;
                }
            }

            value += std::min<uint32>(tightNeighbors, 4) * 6.0f;
            value -= std::min<uint32>(sidePackNeighbors, 5) * 7.0f;
            value -= std::min<uint32>(verticalSplitNeighbors, 4) * 5.0f;

            score.score = value;
            if (score.score > bestCandidate.score)
                bestCandidate = score;
        }

        if (bestCandidate.unit)
        {
            LogMagtheridonDebug(botAI, bot, "trash_pull_selected",
                "source=SelectMagtheridonTrashPullTarget score=" + std::to_string(bestCandidate.score) + " " +
                GetMagtheridonDebugUnit(bot, bestCandidate.unit) +
                " candidates=" + std::to_string(candidates.size()),
                nullptr, 0);
        }
        else
        {
            LogMagtheridonDebug(botAI, bot, "trash_pull_none",
                "source=SelectMagtheridonTrashPullTarget candidates=" + std::to_string(candidates.size()),
                nullptr, 10);
        }

        return bestCandidate.unit;
    }

    bool HasRemainingMagtheridonTrash(PlayerbotAI* botAI, Player* bot)
    {
        if (!botAI || !bot || bot->GetMapId() != MAGTHERIDON_MAP_ID)
            return false;

        std::vector<ObjectGuid> excludedGuids;
        if (Unit* magtheridon = FindMagtheridon(botAI, bot))
            excludedGuids.push_back(magtheridon->GetGUID());

        for (uint32 channelerGuid : {
                 SOUTH_CHANNELER,
                 WEST_CHANNELER,
                 NORTHWEST_CHANNELER,
                 EAST_CHANNELER,
                 NORTHEAST_CHANNELER
             })
        {
            if (Creature* channeler = GetChanneler(bot, channelerGuid))
                excludedGuids.push_back(channeler->GetGUID());
        }

        auto hasTrashInValue = [&](std::string const& valueName) -> bool
        {
            if (Value<GuidVector>* units =
                    botAI->GetAiObjectContext()->GetValue<GuidVector>(valueName))
            {
                for (ObjectGuid const& guid : units->Get())
                {
                    Unit* unit = botAI->GetUnit(guid);
                    Creature* creature = unit ? unit->ToCreature() : nullptr;
                    if (!unit || !creature || !unit->IsAlive())
                        continue;

                    if (unit->GetMapId() != bot->GetMapId())
                        continue;

                    if (std::find(excludedGuids.begin(), excludedGuids.end(), guid) !=
                        excludedGuids.end())
                        continue;

                    if (creature->IsDungeonBoss() || creature->isWorldBoss() ||
                        creature->GetEntry() == NPC_BURNING_ABYSSAL ||
                        creature->GetEntry() == NPC_TARGET_TRIGGER)
                        continue;

                    if (unit->GetDistance2d(MAGTHERIDON_TANK_POSITION.GetPositionX(),
                                            MAGTHERIDON_TANK_POSITION.GetPositionY()) >
                        TRASH_PHASE_SEARCH_RADIUS)
                        continue;

                    if (!AttackersValue::IsPossibleTarget(unit, bot))
                        continue;

                    return true;
                }
            }

            return false;
        };

        return hasTrashInValue("nearest hostile npcs") || hasTrashInValue("nearest npcs");
    }

    bool IsMagtheridonAutoPullReady(PlayerbotAI* botAI, Player* bot)
    {
        if (!botAI || !bot || bot->GetMapId() != MAGTHERIDON_MAP_ID)
            return false;

        Group* group = bot->GetGroup();
        if (!group || !bot->IsAlive() || bot->IsInCombat() || !botAI->IsMainTank(bot))
            return false;

        if (botAI->HasStrategy("stay", BotState::BOT_STATE_NON_COMBAT) ||
            botAI->HasStrategy("passive", BotState::BOT_STATE_NON_COMBAT))
            return false;

        if (Value<std::list<ObjectGuid>>* attackers =
                botAI->GetAiObjectContext()->GetValue<std::list<ObjectGuid>>("attackers"))
        {
            if (!attackers->Get().empty())
                return false;
        }

        Unit* currentTarget = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
        if (currentTarget && currentTarget->IsAlive() && currentTarget->IsInWorld() &&
            currentTarget->GetMapId() == bot->GetMapId())
            return false;

        if (bot->GetHealthPct() < AUTO_PULL_TANK_HP_PCT)
            return false;

        Player* firstHealer = nullptr;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member->GetMapId() != bot->GetMapId())
                continue;

            if (member->GetDistance(bot) > AUTO_PULL_GROUP_RANGE || member->IsInCombat() ||
                member->GetHealthPct() < AUTO_PULL_MEMBER_HP_PCT)
                return false;

            if (!firstHealer && GET_PLAYERBOT_AI(member) && botAI->IsHeal(member))
                firstHealer = member;
        }

        if (firstHealer && firstHealer->GetPower(POWER_MANA) > 0 && firstHealer->GetMaxPower(POWER_MANA) > 0 &&
            firstHealer->GetPowerPct(POWER_MANA) < AUTO_PULL_HEALER_MANA_PCT)
            return false;

        return SelectMagtheridonTrashPullTarget(botAI, bot);
    }

    bool IsMagtheridonEncounterPullAllowed(PlayerbotAI* botAI, Player* bot)
    {
        if (!botAI || !bot || bot->GetMapId() != MAGTHERIDON_MAP_ID)
            return false;

        Unit* magtheridon = FindMagtheridon(botAI, bot);
        if (magtheridon && !magtheridon->HasAura(SPELL_SHADOW_CAGE))
            return true;

        return !HasRemainingMagtheridonTrash(botAI, bot);
    }

    bool IsMagtheridonChannelerPhaseActive(PlayerbotAI* botAI, Player* bot)
    {
        if (!botAI || !bot || bot->GetMapId() != MAGTHERIDON_MAP_ID)
            return false;

        Unit* magtheridon = FindMagtheridon(botAI, bot);
        if (!magtheridon)
            return false;

        if (!magtheridon->HasAura(SPELL_SHADOW_CAGE))
            return true;

        for (uint32 channelerGuid : {
                 SOUTH_CHANNELER,
                 WEST_CHANNELER,
                 NORTHWEST_CHANNELER,
                 EAST_CHANNELER,
                 NORTHEAST_CHANNELER
             })
        {
            if (Creature* channeler = GetChanneler(bot, channelerGuid))
            {
                if (channeler->IsInCombat())
                    return true;
            }
        }

        return false;
    }

    // Identify channelers by their database GUIDs
    Creature* GetChanneler(Player* bot, uint32 dbGuid)
    {
        Map* map = bot->GetMap();
        if (!map)
            return nullptr;

        auto it = map->GetCreatureBySpawnIdStore().find(dbGuid);
        if (it == map->GetCreatureBySpawnIdStore().end())
            return nullptr;

        Creature* channeler = it->second;
        if (!channeler->IsAlive())
            return nullptr;

        return channeler;
    }

    bool HasAliveMagtheridonChanneler(Player* bot)
    {
        if (!bot)
            return false;

        for (uint32 channelerGuid : {
                 SOUTH_CHANNELER,
                 WEST_CHANNELER,
                 NORTHWEST_CHANNELER,
                 EAST_CHANNELER,
                 NORTHEAST_CHANNELER
             })
        {
            if (GetChanneler(bot, channelerGuid))
                return true;
        }

        return false;
    }

    bool HasAliveBurningAbyssal(PlayerbotAI* botAI, Player* bot)
    {
        if (!botAI || !bot)
            return false;

        auto hasAbyssalInValue = [&](std::string const& valueName) -> bool
        {
            if (Value<GuidVector>* units = botAI->GetAiObjectContext()->GetValue<GuidVector>(valueName))
            {
                for (ObjectGuid const& guid : units->Get())
                {
                    Unit* unit = botAI->GetUnit(guid);
                    if (unit && unit->IsAlive() && unit->GetMapId() == bot->GetMapId() &&
                        unit->GetEntry() == NPC_BURNING_ABYSSAL)
                        return true;
                }
            }

            return false;
        };

        return hasAbyssalInValue("nearest hostile npcs") || hasAbyssalInValue("nearest npcs");
    }

    const std::vector<uint32> MANTICRON_CUBE_DB_GUIDS = { 43157, 43158, 43159, 43160, 43161 };

    // Get the positions of all Manticron Cubes by their database GUIDs
    std::vector<CubeInfo> GetAllCubeInfosByDbGuids(Map* map, const std::vector<uint32>& cubeDbGuids)
    {
        std::vector<CubeInfo> cubes;
        if (!map)
            return cubes;

        for (uint32 dbGuid : cubeDbGuids)
        {
            auto bounds = map->GetGameObjectBySpawnIdStore().equal_range(dbGuid);
            if (bounds.first == bounds.second)
                continue;

            GameObject* go = bounds.first->second;
            if (!go)
                continue;

            CubeInfo info;
            info.guid = go->GetGUID();
            info.x = go->GetPositionX();
            info.y = go->GetPositionY();
            info.z = go->GetPositionZ();
            cubes.push_back(info);
        }

        return cubes;
    }

    std::unordered_map<ObjectGuid, CubeInfo> botToCubeAssignment;

    void AssignBotsToCubesByGuidAndCoords(Group* group, const std::vector<CubeInfo>& cubes, PlayerbotAI* botAI)
    {
        botToCubeAssignment.clear();
        if (!group)
            return;

        size_t cubeIndex = 0;
        std::vector<Player*> candidates;

        // Assign ranged DPS (excluding Warlocks) to cubes first
        for (GroupReference* ref = group->GetFirstMember(); ref && cubeIndex < cubes.size(); ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || !botAI->IsRangedDps(member, true) ||
                member->getClass() == CLASS_WARLOCK || !GET_PLAYERBOT_AI(member))
                continue;

            candidates.push_back(member);
            if (candidates.size() >= cubes.size())
                break;
        }

        // If there are still cubes left, assign any other non-tank bots
        if (candidates.size() < cubes.size())
        {
            for (GroupReference* ref = group->GetFirstMember();
                ref && candidates.size() < cubes.size(); ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member) || botAI->IsTank(member))
                    continue;

                if (std::find(candidates.begin(), candidates.end(), member) == candidates.end())
                    candidates.push_back(member);
            }
        }

        for (Player* member : candidates)
        {
            if (cubeIndex >= cubes.size())
                break;

            if (!member || !member->IsAlive())
                continue;

            botToCubeAssignment[member->GetGUID()] = cubes[cubeIndex++];
        }
    }

    std::unordered_map<uint32, bool> lastBlastNovaState;
    std::unordered_map<uint32, time_t> blastNovaTimer;
    std::unordered_map<uint32, time_t> spreadWaitTimer;
    std::unordered_map<uint32, time_t> dpsWaitTimer;

    bool IsSafeFromMagtheridonHazards(PlayerbotAI* botAI, Player* /*bot*/, float x, float y, float /*z*/)
    {
        // Debris
        std::vector<Unit*> debrisHazards;
        const GuidVector npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();
        for (auto const& npcGuid : npcs)
        {
            Unit* unit = botAI->GetUnit(npcGuid);
            if (!unit || unit->GetEntry() != NPC_TARGET_TRIGGER)
                continue;
            debrisHazards.push_back(unit);
        }
        for (Unit* hazard : debrisHazards)
        {
            float dist = hazard->GetDistance2d(x, y);
            if (dist < 9.0f)
                return false;
        }

        // Conflagration
        GuidVector gos = *botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest game objects");
        for (auto const& goGuid : gos)
        {
            GameObject* go = botAI->GetGameObject(goGuid);
            if (!go || go->GetEntry() != GO_BLAZE)
                continue;

            float dist = go->GetDistance2d(x, y);
            if (dist < 5.0f)
                return false;
        }

        return true;
    }
}
