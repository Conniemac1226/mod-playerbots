#include "RaidGruulsLairHelpers.h"
#include "AttackersValue.h"
#include "AiFactory.h"
#include "Group.h"
#include "GroupReference.h"
#include "Playerbots.h"
#include "Value.h"
#include "Unit.h"
#include <algorithm>
#include <ctime>
#include <limits>
#include <string>
#include <unordered_map>

namespace GruulsLairHelpers
{
    std::unordered_map<uint32, time_t> gruulDpsWaitTimer;

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
        constexpr uint32 BOSS_PULL_READY_WINDOW_SECONDS = 45;
        constexpr float MAULGAR_BOSS_TRASH_BLOCK_RADIUS = 32.0f;
        constexpr float GRUUL_BOSS_TRASH_BLOCK_RADIUS = 40.0f;
        constexpr float BOSS_PULL_GROUP_RANGE = 45.0f;
        constexpr float BOSS_PULL_STAGING_RADIUS = 55.0f;
        constexpr time_t BOSS_PULL_DEBUG_THROTTLE_SECONDS = 5;
        constexpr float MAULGAR_FALLBACK_SEARCH_RADIUS = 80.0f;
        constexpr float GRUUL_FALLBACK_SEARCH_RADIUS = 90.0f;

        struct PullCandidateScore
        {
            Unit* unit = nullptr;
            float score = -std::numeric_limits<float>::max();
        };

        std::unordered_map<uint32, time_t> bossPullReadyUntil;
        std::unordered_map<std::string, time_t> bossPullDebugUntil;
    }

    // Olm does not chase properly due to the Core's caster movement issues
    // Thus, the below "OlmTankPosition" is beyond the actual desired tanking location
    // It is the spot to which the OlmTank runs to to pull Olm to a decent tanking location
    // "MaulgarRoomCenter" is to keep healers in a centralized location
    const Position MAULGAR_TANK_POSITION  = {  90.686f, 167.047f, -13.234f };
    const Position OLM_TANK_POSITION      = {  87.485f, 234.942f,  -3.635f };
    const Position BLINDEYE_TANK_POSITION = {  99.681f, 213.989f, -10.345f };
    const Position KROSH_TANK_POSITION    = { 116.880f, 166.208f, -14.231f };
    const Position MAULGAR_ROOM_CENTER    = {  88.754f, 150.759f, -11.569f };
    const Position MAULGAR_HEALER_POSITION = {  89.500f, 197.500f, -10.800f };
    const Position MAULGAR_SUPPORT_POSITION = {  87.000f, 192.000f, -10.500f };
    const Position BLINDEYE_DPS_POSITION   = {  92.000f, 201.500f, -10.400f };
    const Position OLM_DPS_POSITION        = {  84.500f, 208.500f,  -9.000f };
    const Position KIGGLER_TANK_POSITION   = {  64.500f, 166.500f, -14.000f };
    const Position GRUUL_TANK_POSITION    = { 241.238f, 365.025f,  -4.220f };

    Unit* FindHighKingMaulgar(PlayerbotAI* botAI, Player* bot)
    {
        Unit* maulgar = botAI ? botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "high king maulgar")->Get() : nullptr;
        if (maulgar && !maulgar->IsAlive())
            maulgar = nullptr;
        if (maulgar)
            return maulgar;

        if (!botAI || !bot)
            return nullptr;

        auto scan = [&](std::string const& valueName) -> Unit*
        {
            if (Value<GuidVector>* units = botAI->GetAiObjectContext()->GetValue<GuidVector>(valueName))
            {
                for (ObjectGuid const& guid : units->Get())
                {
                    Unit* unit = botAI->GetUnit(guid);
                    if (!unit || !unit->IsAlive() || unit->GetEntry() != 18831)
                        continue;

                    if (unit->GetMapId() != bot->GetMapId())
                        continue;

                    if (unit->GetDistance2d(MAULGAR_SUPPORT_POSITION.GetPositionX(),
                                            MAULGAR_SUPPORT_POSITION.GetPositionY()) >
                        MAULGAR_FALLBACK_SEARCH_RADIUS)
                        continue;

                    return unit;
                }
            }

            return nullptr;
        };

        maulgar = scan("nearest hostile npcs");
        if (!maulgar)
            maulgar = scan("nearest npcs");

        return maulgar;
    }

    Unit* FindGruulTheDragonkiller(PlayerbotAI* botAI, Player* bot)
    {
        Unit* gruul = botAI ? botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "gruul the dragonkiller")->Get() : nullptr;
        if (gruul && !gruul->IsAlive())
            gruul = nullptr;
        if (gruul)
            return gruul;

        if (!botAI || !bot)
            return nullptr;

        auto scan = [&](std::string const& valueName) -> Unit*
        {
            if (Value<GuidVector>* units = botAI->GetAiObjectContext()->GetValue<GuidVector>(valueName))
            {
                for (ObjectGuid const& guid : units->Get())
                {
                    Unit* unit = botAI->GetUnit(guid);
                    Creature* creature = unit ? unit->ToCreature() : nullptr;
                    if (!unit || !unit->IsAlive() ||
                        !creature || (!creature->IsDungeonBoss() && !creature->isWorldBoss()))
                        continue;

                    if (unit->GetMapId() != bot->GetMapId())
                        continue;

                    if (unit->GetDistance2d(GRUUL_TANK_POSITION.GetPositionX(),
                                            GRUUL_TANK_POSITION.GetPositionY()) >
                        GRUUL_FALLBACK_SEARCH_RADIUS)
                        continue;

                    return unit;
                }
            }

            return nullptr;
        };

        gruul = scan("nearest hostile npcs");
        if (!gruul)
            gruul = scan("nearest npcs");

        return gruul;
    }

        bool IsCouncilSupportMember(PlayerbotAI* botAI, Player* self, Player* member)
        {
            if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member))
                return false;

            if (botAI->IsHeal(self))
                return botAI->IsHeal(member);

            return botAI->IsRanged(member) &&
                   !IsKroshMageTank(botAI, member) &&
                   !IsKigglerMoonkinTank(botAI, member);
        }

        Position GetMaulgarCouncilAnchor(PlayerbotAI* botAI, Player* bot, Unit* focusTarget)
        {
            if (botAI->IsHeal(bot))
                return MAULGAR_HEALER_POSITION;

            Unit* blindeye =
                botAI->GetAiObjectContext()->GetValue<Unit*>(
                    "find target", "blindeye the seer")->Get();
            if (focusTarget && blindeye &&
                focusTarget->GetGUID() == blindeye->GetGUID())
                return BLINDEYE_DPS_POSITION;

            Unit* olm =
                botAI->GetAiObjectContext()->GetValue<Unit*>(
                    "find target", "olm the summoner")->Get();
            if (focusTarget && olm &&
                focusTarget->GetGUID() == olm->GetGUID())
                return OLM_DPS_POSITION;

            return MAULGAR_SUPPORT_POSITION;
        }

        bool IsGruulsLairMap(Player* bot)
        {
            return bot && bot->GetMapId() == GRUULS_LAIR_MAP_ID;
        }

        Unit* GetAliveTarget(PlayerbotAI* botAI, std::string const& name)
        {
            Unit* unit = botAI ? botAI->GetAiObjectContext()->GetValue<Unit*>("find target", name)->Get() : nullptr;
            return unit && unit->IsAlive() ? unit : nullptr;
        }

        bool IsBossUnit(Unit* unit)
        {
            Creature* creature = unit ? unit->ToCreature() : nullptr;
            return creature && (creature->IsDungeonBoss() || creature->isWorldBoss());
        }

        float GetFacingDelta(Player* bot, Unit* unit)
        {
            float delta = Position::NormalizeOrientation(bot->GetAngle(unit) - bot->GetOrientation());
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
                float const mid = (AUTO_PULL_IDEAL_MIN_RANGE + AUTO_PULL_IDEAL_MAX_RANGE) * 0.5f;
                return 34.0f - std::abs(distance - mid) * 1.15f;
            }

            return 14.0f - (distance - AUTO_PULL_IDEAL_MAX_RANGE) * 1.35f;
        }

        std::vector<Unit*> CollectGruulsLairPullCandidates(PlayerbotAI* botAI, Player* bot)
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
                        if (!unit || !unit->IsAlive() || unit->IsInCombat() || IsBossUnit(unit))
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

        PullCandidateScore ScoreGruulsLairPullTarget(
            Player* bot, Unit* candidate, std::vector<Unit*> const& candidates)
        {
            PullCandidateScore result;
            if (!bot || !candidate)
                return result;

            result.unit = candidate;

            float const distance3d = bot->GetDistance(candidate);
            float const distance2d = bot->GetExactDist2d(candidate);
            float const heightDelta = std::abs(bot->GetPositionZ() - candidate->GetPositionZ());
            float const facingDelta = GetFacingDelta(bot, candidate);

            float score = 0.0f;
            score += ScorePullDistance(distance3d);

            float const forwardRatio = 1.0f -
                std::min(facingDelta, static_cast<float>(M_PI)) /
                static_cast<float>(M_PI);
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

            score += std::min<uint32>(tightNeighbors, 4) * 6.0f;
            score -= std::min<uint32>(sidePackNeighbors, 5) * 7.0f;
            score -= std::min<uint32>(verticalSplitNeighbors, 4) * 5.0f;

            result.score = score;
            return result;
        }

        bool IsTrashNearPosition(Unit* trash, Position const& position, float radius)
        {
            return trash &&
                   trash->GetDistance2d(position.GetPositionX(), position.GetPositionY()) <= radius;
        }

        bool HasMaulgarAreaBlockingTrash(PlayerbotAI* botAI, Player* bot)
        {
            std::vector<Unit*> candidates = CollectGruulsLairPullCandidates(botAI, bot);
            if (candidates.empty())
                return false;

            Unit* councilBosses[] =
            {
                FindHighKingMaulgar(botAI, bot),
                GetAliveTarget(botAI, "kiggler the crazed"),
                GetAliveTarget(botAI, "krosh firehand"),
                GetAliveTarget(botAI, "olm the summoner"),
                GetAliveTarget(botAI, "blindeye the seer")
            };

            for (Unit* candidate : candidates)
            {
                if (!candidate)
                    continue;

                for (Unit* boss : councilBosses)
                {
                    if (boss && candidate->GetDistance2d(boss) <= MAULGAR_BOSS_TRASH_BLOCK_RADIUS)
                        return true;
                }

                if (IsTrashNearPosition(candidate, MAULGAR_TANK_POSITION, MAULGAR_BOSS_TRASH_BLOCK_RADIUS) ||
                    IsTrashNearPosition(candidate, MAULGAR_SUPPORT_POSITION, MAULGAR_BOSS_TRASH_BLOCK_RADIUS) ||
                    IsTrashNearPosition(candidate, MAULGAR_HEALER_POSITION, MAULGAR_BOSS_TRASH_BLOCK_RADIUS))
                    return true;
            }

            return false;
        }

        bool HasGruulAreaBlockingTrash(PlayerbotAI* botAI, Player* bot)
        {
            std::vector<Unit*> candidates = CollectGruulsLairPullCandidates(botAI, bot);
            if (candidates.empty())
                return false;

            Unit* gruul = FindGruulTheDragonkiller(botAI, bot);
            for (Unit* candidate : candidates)
            {
                if (!candidate)
                    continue;

                if (gruul && candidate->GetDistance2d(gruul) <= GRUUL_BOSS_TRASH_BLOCK_RADIUS)
                    return true;

                if (IsTrashNearPosition(candidate, GRUUL_TANK_POSITION, GRUUL_BOSS_TRASH_BLOCK_RADIUS))
                    return true;
            }

            return false;
        }

        void LogBossPullDebug(PlayerbotAI* botAI, Player* bot, std::string const& bossKey,
                              std::string const& reason)
        {
            if (!botAI || !bot || !botAI->IsMainTank(bot) || !bot->GetMap())
                return;

            std::string throttleKey = std::to_string(bot->GetMap()->GetInstanceId()) + ":" + bossKey + ":" + reason;
            time_t now = time(nullptr);
            auto it = bossPullDebugUntil.find(throttleKey);
            if (it != bossPullDebugUntil.end() && it->second > now)
                return;

            bossPullDebugUntil[throttleKey] = now + BOSS_PULL_DEBUG_THROTTLE_SECONDS;
            bool readyLatchActive = false;
            auto readyIt = bossPullReadyUntil.find(bot->GetMap()->GetInstanceId());
            if (readyIt != bossPullReadyUntil.end() && readyIt->second >= now)
                readyLatchActive = true;

            LOG_INFO("playerbots",
                     "GL-DEBUG boss={} mt={} reason={} hp={} attackers={} ready_latch={}",
                     bossKey, bot->GetName(), reason, bot->GetHealthPct(),
                     botAI->GetAiObjectContext() ? botAI->GetAiObjectContext()->GetValue<uint8>("attacker count")->Get() : 255,
                     readyLatchActive ? 1 : 0);
        }

        bool IsBossPullFallbackReady(PlayerbotAI* botAI, Player* bot, Unit* boss,
                                     Position const& stagingPosition, float blockingTrashRadius)
        {
            if (!botAI || !bot || !boss || !bot->GetGroup() || !bot->IsAlive() ||
                bot->IsInCombat() || !botAI->IsMainTank(bot))
                return false;

            if (botAI->HasStrategy("stay", BOT_STATE_NON_COMBAT) ||
                botAI->HasStrategy("passive", BOT_STATE_NON_COMBAT))
                return false;

            if (boss->IsInCombat())
                return true;

            AiObjectContext* context = botAI->GetAiObjectContext();
            if (!context || context->GetValue<uint8>("attacker count")->Get() != 0)
                return false;

            if (bot->HealthBelowPct(AUTO_PULL_TANK_HP_PCT))
                return false;

            if (bot->GetDistance2d(stagingPosition.GetPositionX(), stagingPosition.GetPositionY()) >
                    BOSS_PULL_STAGING_RADIUS &&
                bot->GetDistance2d(boss) > BOSS_PULL_STAGING_RADIUS)
                return false;

            if (blockingTrashRadius == MAULGAR_BOSS_TRASH_BLOCK_RADIUS)
            {
                if (HasMaulgarAreaBlockingTrash(botAI, bot))
                    return false;
            }
            else if (HasGruulAreaBlockingTrash(botAI, bot))
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

                if (member->GetMapId() != bot->GetMapId())
                    return false;

                if (member->GetDistance2d(stagingPosition.GetPositionX(), stagingPosition.GetPositionY()) >
                        BOSS_PULL_GROUP_RANGE &&
                    member->GetDistance2d(boss) > BOSS_PULL_GROUP_RANGE)
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

        bool IsBossPullReadyActive(Player* bot)
        {
            if (!bot)
                return false;

            auto it = bossPullReadyUntil.find(bot->GetMap()->GetInstanceId());
            if (it == bossPullReadyUntil.end())
                return false;

            time_t now = time(nullptr);
            if (it->second < now)
            {
                bossPullReadyUntil.erase(it);
                return false;
            }

            return true;
        }

        bool IsAnyCouncilBossInCombatInternal(PlayerbotAI* botAI)
        {
            const char* councilBossNames[] =
            {
                "high king maulgar",
                "kiggler the crazed",
                "krosh firehand",
                "olm the summoner",
                "blindeye the seer"
            };

            for (char const* name : councilBossNames)
            {
                if (Unit* boss = GetAliveTarget(botAI, name))
                {
                    if (boss->IsInCombat())
                        return true;
                }
            }

            return false;
        }

    bool IsAnyOgreBossAlive(PlayerbotAI* botAI)
    {
        const char* ogreBossNames[] =
        {
            "high king maulgar",
            "kiggler the crazed",
            "krosh firehand",
            "olm the summoner",
            "blindeye the seer"
        };

        for (const char* name : ogreBossNames)
        {
            Unit* boss = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", name)->Get();
            if (!boss || !boss->IsAlive())
                continue;
            return true;
        }

        return false;
    }

    bool IsHighKingMaulgarPullAllowed(PlayerbotAI* botAI, Player* bot)
    {
        if (!GruulsLairHelpers::IsGruulsLairMap(bot))
            return false;

        if (GruulsLairHelpers::IsAnyCouncilBossInCombat(botAI))
            return true;

        Unit* maulgar = GruulsLairHelpers::FindHighKingMaulgar(botAI, bot);
        Unit* kiggler = GruulsLairHelpers::GetAliveTarget(botAI, "kiggler the crazed");
        Unit* krosh = GruulsLairHelpers::GetAliveTarget(botAI, "krosh firehand");
        Unit* olm = GruulsLairHelpers::GetAliveTarget(botAI, "olm the summoner");
        Unit* blindeye = GruulsLairHelpers::GetAliveTarget(botAI, "blindeye the seer");

        if (!maulgar && !kiggler && !krosh && !olm && !blindeye)
            return false;

        Unit* anchorBoss = maulgar ? maulgar :
            (blindeye ? blindeye : (olm ? olm : (krosh ? krosh : kiggler)));
        if (botAI->IsMainTank(bot) && anchorBoss &&
            bot->GetDistance2d(MAULGAR_SUPPORT_POSITION.GetPositionX(), MAULGAR_SUPPORT_POSITION.GetPositionY()) <=
                BOSS_PULL_STAGING_RADIUS)
        {
            if (!IsBossPullReadyActive(bot))
                LogBossPullDebug(botAI, bot, "maulgar", "ready_latch_inactive");

            if (HasMaulgarAreaBlockingTrash(botAI, bot))
                LogBossPullDebug(botAI, bot, "maulgar", "boss_area_trash_block");
        }

        if (GruulsLairHelpers::IsBossPullReadyActive(bot) &&
            !HasMaulgarAreaBlockingTrash(botAI, bot))
            return true;

        bool fallbackReady = IsBossPullFallbackReady(botAI, bot, anchorBoss, MAULGAR_SUPPORT_POSITION,
                                                     MAULGAR_BOSS_TRASH_BLOCK_RADIUS);
        if (!fallbackReady && botAI->IsMainTank(bot) && anchorBoss &&
            bot->GetDistance2d(MAULGAR_SUPPORT_POSITION.GetPositionX(), MAULGAR_SUPPORT_POSITION.GetPositionY()) <=
                BOSS_PULL_STAGING_RADIUS)
            LogBossPullDebug(botAI, bot, "maulgar", "fallback_not_ready");

        return fallbackReady;
    }

    bool IsGruulTheDragonkillerPullAllowed(PlayerbotAI* botAI, Player* bot)
    {
        if (!GruulsLairHelpers::IsGruulsLairMap(bot))
            return false;

        Unit* gruul = GruulsLairHelpers::FindGruulTheDragonkiller(botAI, bot);
        if (gruul)
        {
            if (gruul->IsInCombat())
                return true;
        }

        if (!gruul)
            return false;

        if (botAI->IsMainTank(bot) &&
            bot->GetDistance2d(GRUUL_TANK_POSITION.GetPositionX(), GRUUL_TANK_POSITION.GetPositionY()) <=
                BOSS_PULL_STAGING_RADIUS)
        {
            if (!IsBossPullReadyActive(bot))
                LogBossPullDebug(botAI, bot, "gruul", "ready_latch_inactive");

            if (HasGruulAreaBlockingTrash(botAI, bot))
                LogBossPullDebug(botAI, bot, "gruul", "boss_area_trash_block");
        }

        if (GruulsLairHelpers::IsBossPullReadyActive(bot) &&
            !HasGruulAreaBlockingTrash(botAI, bot))
            return true;

        bool fallbackReady = IsBossPullFallbackReady(botAI, bot, gruul, GRUUL_TANK_POSITION,
                                                     GRUUL_BOSS_TRASH_BLOCK_RADIUS);
        if (!fallbackReady && botAI->IsMainTank(bot) &&
            bot->GetDistance2d(GRUUL_TANK_POSITION.GetPositionX(), GRUUL_TANK_POSITION.GetPositionY()) <=
                BOSS_PULL_STAGING_RADIUS)
            LogBossPullDebug(botAI, bot, "gruul", "fallback_not_ready");

        return fallbackReady;
    }

    bool IsKroshMageTank(PlayerbotAI* botAI, Player* bot)
    {
        Group* group = bot->GetGroup();
        if (!group)
            return false;

        // (1) First loop: Return the first assistant Mage (real player or bot)
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member->getClass() != CLASS_MAGE)
                continue;

            if (group->IsAssistant(member->GetGUID()))
                return member == bot;
        }

        // (2) Fall back to bot Mage with highest HP
        Player* highestHpMage = nullptr;
        uint32 highestHp = 0;

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member) ||
                member->getClass() != CLASS_MAGE)
                continue;

            uint32 hp = member->GetMaxHealth();
            if (!highestHpMage || hp > highestHp)
            {
                highestHpMage = member;
                highestHp = hp;
            }
        }

        // (3) Return the found Mage tank, or nullptr if none found
        return highestHpMage == bot;
    }

    bool IsKigglerMoonkinTank(PlayerbotAI* botAI, Player* bot)
    {
        Group* group = bot->GetGroup();
        if (!group)
            return false;

        // (1) First loop: Return the first assistant Moonkin (real player or bot)
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member->getClass() != CLASS_DRUID)
                continue;

            if (group->IsAssistant(member->GetGUID()) &&
                AiFactory::GetPlayerSpecTab(member) == DRUID_TAB_BALANCE)
                return member == bot;
        }

        // (2) Fall back to bot Moonkin with highest HP
        Player* highestHpMoonkin = nullptr;
        uint32 highestHp = 0;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member->getClass() != CLASS_DRUID ||
                !GET_PLAYERBOT_AI(member) || AiFactory::GetPlayerSpecTab(member) != DRUID_TAB_BALANCE)
                continue;

            uint32 hp = member->GetMaxHealth();
            if (!highestHpMoonkin || hp > highestHp)
            {
                highestHpMoonkin = member;
                highestHp = hp;
            }
        }

        // (3) Return the found Moonkin tank, or nullptr if none found
        return highestHpMoonkin == bot;
    }

    bool IsAnyCouncilBossInCombat(PlayerbotAI* botAI)
    {
        return IsAnyCouncilBossInCombatInternal(botAI);
    }

    bool IsPositionSafe(PlayerbotAI* botAI, Player* bot, Position pos)
    {
        const float KROSH_SAFE_DISTANCE = 20.0f;
        const float KIGGLER_SAFE_DISTANCE = 30.0f;
        const float MAULGAR_SAFE_DISTANCE = 10.0f;
        bool isSafe = true;

        Unit* krosh = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "krosh firehand")->Get();
        if (krosh && krosh->IsAlive())
        {
            float dist = krosh->GetDistance2d(pos.GetPositionX(), pos.GetPositionY());
            if (dist < KROSH_SAFE_DISTANCE)
                isSafe = false;
        }

        Unit* maulgar = FindHighKingMaulgar(botAI, bot);
        if (botAI->IsRanged(bot) && maulgar && maulgar->IsAlive())
        {
            float dist = maulgar->GetDistance2d(pos.GetPositionX(), pos.GetPositionY());
            if (dist < MAULGAR_SAFE_DISTANCE)
                isSafe = false;
        }

        Unit* kiggler = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "kiggler the crazed")->Get();
        if (!IsKigglerMoonkinTank(botAI, bot) && kiggler && kiggler->IsAlive())
        {
            float dist = kiggler->GetDistance2d(pos.GetPositionX(), pos.GetPositionY());
            if (dist < KIGGLER_SAFE_DISTANCE)
                isSafe = false;
        }

        return isSafe;
    }

    bool TryGetNewSafePosition(PlayerbotAI* botAI, Player* bot, Position& outPos)
    {
        const float SEARCH_RADIUS = 30.0f;
        const uint8 NUM_POSITIONS = 32;

        outPos = { bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ() };
        if (IsPositionSafe(botAI, bot, outPos))
        {
            outPos = Position();
            return false;
        }

        float bestScore = std::numeric_limits<float>::max();
        bool foundSafeSpot = false;
        Position bestPos;

        for (int i = 0; i < NUM_POSITIONS; ++i)
        {
            float angle = 2 * M_PI * i / NUM_POSITIONS;
            Position candidatePos;
            candidatePos.Relocate(bot->GetPositionX() + SEARCH_RADIUS * cos(angle),
                                  bot->GetPositionY() + SEARCH_RADIUS * sin(angle),
                                  bot->GetPositionZ());

            float destX = candidatePos.GetPositionX();
            float destY = candidatePos.GetPositionY();
            float destZ = candidatePos.GetPositionZ();
            if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                bot->GetPositionZ(), destX, destY, destZ, true))
                continue;

            if (destX != candidatePos.GetPositionX() || destY != candidatePos.GetPositionY())
                continue;

            candidatePos.Relocate(destX, destY, destZ);

            if (IsPositionSafe(botAI, bot, candidatePos))
            {
                float movementDistance = bot->GetDistance2d(destX, destY);
                if (movementDistance < bestScore)
                {
                    bestScore = movementDistance;
                    bestPos = candidatePos;
                    foundSafeSpot = true;
                }
            }
        }

        if (foundSafeSpot)
        {
            outPos = bestPos;
            return true;
        }

        outPos = Position();
        return false;
    }

    bool TryGetMaulgarCouncilPosition(PlayerbotAI* botAI, Player* bot, Unit* focusTarget, Position& outPos)
    {
        Group* group = bot->GetGroup();
        if (!group || (!botAI->IsHeal(bot) && !botAI->IsRanged(bot)))
        {
            outPos = Position();
            return false;
        }

        std::vector<Player*> spreadMembers;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (GruulsLairHelpers::IsCouncilSupportMember(botAI, bot, member))
                spreadMembers.push_back(member);
        }

        if (spreadMembers.empty())
        {
            outPos = Position();
            return false;
        }

        auto it = std::find(spreadMembers.begin(), spreadMembers.end(), bot);
        size_t botIndex = (it != spreadMembers.end()) ?
            static_cast<size_t>(std::distance(spreadMembers.begin(), it)) : 0;
        size_t count = spreadMembers.size();

        const Position anchor = GruulsLairHelpers::GetMaulgarCouncilAnchor(
            botAI, bot, focusTarget);
        const float radius = botAI->IsHeal(bot) ? 6.0f : 8.0f;
        const float angle = (count <= 1) ? 0.0f :
            (2.0f * static_cast<float>(M_PI) * static_cast<float>(botIndex) /
             static_cast<float>(count));

        Position candidate;
        candidate.Relocate(anchor.GetPositionX() + radius * std::cos(angle),
                           anchor.GetPositionY() + radius * std::sin(angle),
                           anchor.GetPositionZ());

        float currentDistToCandidate = bot->GetExactDist2d(candidate.GetPositionX(), candidate.GetPositionY());
        if (currentDistToCandidate <= 2.0f &&
            GruulsLairHelpers::IsPositionSafe(botAI, bot, candidate))
        {
            outPos = Position();
            return false;
        }

        constexpr uint8 numAngles = 16;
        const float searchRadii[] = { radius, radius + 4.0f, radius + 8.0f };
        Position bestPos;
        float bestScore = std::numeric_limits<float>::max();
        bool found = false;

        for (float searchRadius : searchRadii)
        {
            for (uint8 i = 0; i < numAngles; ++i)
            {
                float testAngle = angle +
                    (2.0f * static_cast<float>(M_PI) * static_cast<float>(i) /
                     static_cast<float>(numAngles));
                float destX = anchor.GetPositionX() + searchRadius * std::cos(testAngle);
                float destY = anchor.GetPositionY() + searchRadius * std::sin(testAngle);
                float destZ = anchor.GetPositionZ();

                if (!bot->GetMap()->CheckCollisionAndGetValidCoords(
                        bot, bot->GetPositionX(), bot->GetPositionY(),
                        bot->GetPositionZ(), destX, destY, destZ, true))
                    continue;

                Position testPos;
                testPos.Relocate(destX, destY, destZ);
                if (!GruulsLairHelpers::IsPositionSafe(botAI, bot, testPos))
                    continue;

                float score = std::abs(searchRadius - radius) +
                    candidate.GetExactDist2d(destX, destY) +
                    (bot->GetDistance2d(destX, destY) * 0.25f);
                if (score < bestScore)
                {
                    bestScore = score;
                    bestPos = testPos;
                    found = true;
                }
            }
        }

        if (!found)
        {
            outPos = Position();
            return false;
        }

        outPos = bestPos;
        return true;
    }

    bool IsGruulsLairAutoPullReady(PlayerbotAI* botAI, Player* bot)
    {
        if (!botAI || !GruulsLairHelpers::IsGruulsLairMap(bot) || !bot ||
            !bot->GetGroup() ||
            !bot->IsAlive() || bot->IsInCombat() || !botAI->IsMainTank(bot))
            return false;

        if (botAI->HasStrategy("stay", BOT_STATE_NON_COMBAT) ||
            botAI->HasStrategy("passive", BOT_STATE_NON_COMBAT))
            return false;

        AiObjectContext* context = botAI->GetAiObjectContext();
        if (!context || context->GetValue<uint8>("attacker count")->Get() != 0)
            return false;

        if (Unit* currentTarget = context->GetValue<Unit*>("current target")->Get())
        {
            if (currentTarget->IsAlive() && currentTarget->IsInWorld() &&
                currentTarget->GetMapId() == bot->GetMapId())
                return false;
        }

        if (bot->HealthBelowPct(GruulsLairHelpers::AUTO_PULL_TANK_HP_PCT))
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

            if (bot->GetMapId() != member->GetMapId() ||
                bot->GetDistance(member) > GruulsLairHelpers::AUTO_PULL_GROUP_RANGE)
                return false;

            if (member->HealthBelowPct(GruulsLairHelpers::AUTO_PULL_MEMBER_HP_PCT))
                return false;

            if (!foundHealer && botAI->IsHeal(member))
            {
                foundHealer = true;
                healerReady = member->getPowerType() != POWER_MANA ||
                    member->GetPowerPct(POWER_MANA) >=
                    GruulsLairHelpers::AUTO_PULL_HEALER_MANA_PCT;
            }
        }

        return (!foundHealer || healerReady) &&
               GruulsLairHelpers::SelectGruulsLairTrashPullTarget(botAI, bot);
    }

    Unit* SelectGruulsLairTrashPullTarget(PlayerbotAI* botAI, Player* bot)
    {
        if (!botAI || !bot)
            return nullptr;

        std::vector<Unit*> candidates =
            GruulsLairHelpers::CollectGruulsLairPullCandidates(botAI, bot);
        GruulsLairHelpers::PullCandidateScore bestCandidate;

        for (Unit* candidate : candidates)
        {
            GruulsLairHelpers::PullCandidateScore scored =
                GruulsLairHelpers::ScoreGruulsLairPullTarget(
                    bot, candidate, candidates);
            if (scored.score > bestCandidate.score)
                bestCandidate = scored;
        }

        return bestCandidate.score > 0.0f ? bestCandidate.unit : nullptr;
    }

    void MarkGruulsLairBossPullReady(PlayerbotAI* /*botAI*/, Player* bot)
    {
        if (!bot || !bot->GetMap() || !GruulsLairHelpers::IsGruulsLairMap(bot))
            return;

        GruulsLairHelpers::bossPullReadyUntil[bot->GetMap()->GetInstanceId()] =
            time(nullptr) + GruulsLairHelpers::BOSS_PULL_READY_WINDOW_SECONDS;
    }
}
