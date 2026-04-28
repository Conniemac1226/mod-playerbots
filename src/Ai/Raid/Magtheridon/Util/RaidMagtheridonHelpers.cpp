#include "RaidMagtheridonHelpers.h"
#include "AttackersValue.h"
#include "Creature.h"
#include "GameObject.h"
#include "Map.h"
#include "ObjectGuid.h"
#include "Playerbots.h"
#include "Value.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace MagtheridonHelpers
{
    namespace
    {
        constexpr float MAGTHERIDON_FALLBACK_SEARCH_RADIUS = 90.0f;
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
                    if (!unit || !unit->IsAlive() || unit->IsInCombat())
                        continue;

                    if (unit->GetMapId() != bot->GetMapId())
                        continue;

                    if (std::find(excludedGuids.begin(), excludedGuids.end(), guid) != excludedGuids.end())
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

    bool IsSafeFromMagtheridonHazards(PlayerbotAI* botAI, Player* bot, float x, float y, float z)
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
