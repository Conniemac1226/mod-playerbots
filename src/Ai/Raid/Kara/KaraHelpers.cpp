#include "KaraHelpers.h"
#include "Playerbots.h"
#include "InstanceScript.h"
#include "PathGenerator.h"
#include <cmath>
#include <unordered_set>

namespace KarazhanHelpers
{
    namespace
    {
        bool HasChessSessionAura(Group* group)
        {
            if (!group)
                return false;

            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (member && member->IsAlive() && member->HasAura(SPELL_GAME_IN_SESSION))
                    return true;
            }

            return false;
        }

        struct ChessNearbyCache
        {
            uint32 stampMs = 0;
            std::vector<Creature*> pieces;
            std::vector<Creature*> friendlyPieces;
            std::vector<Creature*> moveTriggers;
            bool hasChessPieceNearby = false;
            bool hasCombatPieceNearby = false;
            bool hasMedivhHazardNearby = false;
        };

        static std::unordered_map<ObjectGuid, ChessNearbyCache> chessNearbyCacheByBot;
        static constexpr uint32 ChessNearbyCacheTtlMs = 250;

        static bool IsFreshCache(uint32 nowMs, uint32 stampMs, uint32 ttlMs)
        {
            return stampMs != 0 && (nowMs - stampMs) <= ttlMs;
        }

        static void CollectNearbyChessData(PlayerbotAI* botAI, Player* bot, ChessNearbyCache& cache, size_t& scannedCount)
        {
            cache.pieces.clear();
            cache.friendlyPieces.clear();
            cache.moveTriggers.clear();
            cache.hasChessPieceNearby = false;
            cache.hasCombatPieceNearby = false;
            cache.hasMedivhHazardNearby = false;
            scannedCount = 0;

            if (!botAI || !bot)
                return;

            GuidVector const npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();
            scannedCount += npcs.size();

            std::unordered_set<ObjectGuid::LowType> seenTriggers;
            for (ObjectGuid const& npcGuid : npcs)
            {
                Creature* creature = botAI->GetCreature(npcGuid);
                if (!creature || !creature->IsInWorld() || !creature->IsAlive())
                    continue;

                if (IsChessPieceEntry(creature->GetEntry()))
                {
                    cache.hasChessPieceNearby = true;
                    cache.pieces.push_back(creature);
                    if (IsFriendlyChessPieceForBot(bot, creature))
                        cache.friendlyPieces.push_back(creature);
                    if (creature->IsInCombat() || creature->GetVictim())
                        cache.hasCombatPieceNearby = true;
                }
                else if (creature->GetEntry() == 22519)
                {
                    if (seenTriggers.insert(creature->GetGUID().GetCounter()).second)
                        cache.moveTriggers.push_back(creature);
                }
                else if (creature->GetEntry() == NPC_CHESS_EVENT_MEDIVH_CHEAT_FIRES)
                {
                    if (creature->GetExactDist2d(bot) <= 100.0f)
                        cache.hasMedivhHazardNearby = true;
                }
            }

            WorldObject* anchor = bot;
            if (Unit* charm = bot->GetCharm())
                anchor = charm;

            std::list<Creature*> gridTriggers;
            anchor->GetCreatureListWithEntryInGrid(gridTriggers, 22519, 120.0f);
            scannedCount += gridTriggers.size();
            for (Creature* creature : gridTriggers)
            {
                if (!creature)
                    continue;
                if (!seenTriggers.insert(creature->GetGUID().GetCounter()).second)
                    continue;
                cache.moveTriggers.push_back(creature);
            }
        }

        static ChessNearbyCache const& GetChessNearbyCache(PlayerbotAI* botAI, Player* bot, size_t& scannedCount, bool& usedCache)
        {
            static ChessNearbyCache empty;
            usedCache = false;

            if (!botAI || !bot || bot->GetMapId() != KARAZHAN_MAP_ID || !IsInsideChessFightArea(Position(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ())))
                return empty;

            uint32 const nowMs = getMSTime();
            ChessNearbyCache& cache = chessNearbyCacheByBot[bot->GetGUID()];
            if (IsFreshCache(nowMs, cache.stampMs, ChessNearbyCacheTtlMs))
            {
                usedCache = true;
                scannedCount = 0;
                return cache;
            }

            size_t scanned = 0;
            CollectNearbyChessData(botAI, bot, cache, scanned);
            cache.stampMs = nowMs;
            scannedCount = scanned;
            return cache;
        }

    }

    // Attumen the Huntsman
    std::unordered_map<uint32, time_t> attumenDpsWaitTimer;
    // Big Bad Wolf
    std::unordered_map<ObjectGuid, uint8> bigBadWolfRunIndex;
    // Netherspite
    std::unordered_map<uint32, time_t> netherspiteDpsWaitTimer;
    std::unordered_map<ObjectGuid, time_t> redBeamMoveTimer;
    std::unordered_map<ObjectGuid, bool> lastBeamMoveSideways;
    // Nightbane
    std::unordered_map<uint32, time_t> nightbaneDpsWaitTimer;
    std::unordered_map<ObjectGuid, uint8> nightbaneTankStep;
    std::unordered_map<ObjectGuid, uint8> nightbaneRangedStep;
    std::unordered_map<uint32, time_t> nightbaneFlightPhaseStartTimer;
    std::unordered_map<uint32, bool> nightbaneWasInFlightPhase;
    std::unordered_map<ObjectGuid, bool> nightbaneRainOfBonesHit;
    std::unordered_map<ObjectGuid, ObjectGuid> chessAssignedPieceByBot;
    std::unordered_map<ObjectGuid, ObjectGuid> chessAssignedBotByPiece;
    std::unordered_map<ObjectGuid, time_t> chessAssignmentLockUntil;
    std::unordered_map<ObjectGuid, time_t> chessSelfAbilityThrottleByPiece;

    const Position MAIDEN_OF_VIRTUE_BOSS_POSITION = { -10945.881f, -2103.782f, 92.712f };
    const Position MAIDEN_OF_VIRTUE_RANGED_POSITION[8] =
    {
        { -10931.178f, -2116.580f, 92.179f },
        { -10925.828f, -2102.425f, 92.180f },
        { -10933.089f, -2088.502f, 92.180f },
        { -10947.590f, -2082.815f, 92.180f },
        { -10960.912f, -2090.437f, 92.179f },
        { -10966.017f, -2105.288f, 92.175f },
        { -10959.242f, -2119.617f, 92.180f },
        { -10944.495f, -2123.857f, 92.180f },
    };

    const Position BIG_BAD_WOLF_BOSS_POSITION = { -10913.391f, -1773.508f, 90.477f };
    const Position BIG_BAD_WOLF_RUN_POSITION[4] =
    {
        { -10875.456f, -1779.036f, 90.477f },
        { -10872.281f, -1751.638f, 90.477f },
        { -10910.492f, -1747.401f, 90.477f },
        { -10913.391f, -1773.508f, 90.477f },
    };

    const Position THE_CURATOR_BOSS_POSITION = { -11139.463f, -1884.645f, 165.765f };

    const Position NIGHTBANE_TRANSITION_BOSS_POSITION = { -11160.646f, -1932.773f, 91.473f }; // near some ribs
    const Position NIGHTBANE_FINAL_BOSS_POSITION = { -11173.530f, -1940.707f, 91.473f };
    const Position NIGHTBANE_RANGED_POSITION1 = { -11145.949f, -1970.927f, 91.473f };
    const Position NIGHTBANE_RANGED_POSITION2 = { -11143.594f, -1954.981f, 91.473f };
    const Position NIGHTBANE_RANGED_POSITION3 = { -11159.778f, -1961.031f, 91.473f };
    const Position NIGHTBANE_FLIGHT_STACK_POSITION = { -11159.555f, -1893.526f, 91.473f }; // Broken Barrel
    const Position NIGHTBANE_RAIN_OF_BONES_POSITION = { -11165.233f, -1911.123f, 91.473f };

    Unit* GetFirstAliveUnit(const std::vector<Unit*>& units)
    {
        for (Unit* unit : units)
        {
            if (unit && unit->IsAlive())
                return unit;
        }

        return nullptr;
    }

    bool IsFlameWreathActive(PlayerbotAI* botAI, Player* bot)
    {
        Unit* aran = botAI->GetAiObjectContext()->GetValue<Unit*>("find target", "shade of aran")->Get();
        Spell* currentSpell = aran ? aran->GetCurrentSpell(CURRENT_GENERIC_SPELL) : nullptr;

        if (currentSpell && currentSpell->m_spellInfo &&
            currentSpell->m_spellInfo->Id == SPELL_FLAME_WREATH_CAST)
            return true;

        if (Group* group = bot->GetGroup())
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (!member || !member->IsAlive())
                    continue;

                if (member->HasAura(SPELL_FLAME_WREATH_AURA))
                    return true;
            }
        }

        return false;
    }

    // Red beam blockers: tank bots, no Nether Exhaustion Red
    std::vector<Player*> GetRedBlockers(PlayerbotAI* botAI, Player* bot)
    {
        std::vector<Player*> redBlockers;
        if (Group* group = bot->GetGroup())
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (!member || !member->IsAlive() || !botAI->IsTank(member) || !GET_PLAYERBOT_AI(member) ||
                    member->HasAura(SPELL_NETHER_EXHAUSTION_RED))
                    continue;

                redBlockers.push_back(member);
            }
        }

        return redBlockers;
    }

    // Blue beam blockers: non-Rogue/Warrior DPS bots, no Nether Exhaustion Blue and <24 stacks of Blue Beam debuff
    std::vector<Player*> GetBlueBlockers(PlayerbotAI* botAI, Player* bot)
    {
        std::vector<Player*> blueBlockers;
        if (Group* group = bot->GetGroup())
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member))
                    continue;

                bool hasExhaustion = member->HasAura(SPELL_NETHER_EXHAUSTION_BLUE);
                Aura* blueBuff = member->GetAura(SPELL_BLUE_BEAM_DEBUFF);
                bool overStack = blueBuff && blueBuff->GetStackAmount() >= 24;

                bool isDps = botAI->IsDps(member);
                bool isWarrior = member->getClass() == CLASS_WARRIOR;
                bool isRogue = member->getClass() == CLASS_ROGUE;

                if (isDps && !isWarrior && !isRogue && !hasExhaustion && !overStack)
                    blueBlockers.push_back(member);
            }
        }

        return blueBlockers;
    }

    // Green beam blockers:
    // (1) Prioritize Rogues and non-tank Warrior bots, no Nether Exhaustion Green
    // (2) Then assign Healer bots, no Nether Exhaustion Green and <24 stacks of Green Beam debuff
    std::vector<Player*> GetGreenBlockers(PlayerbotAI* botAI, Player* bot)
    {
        std::vector<Player*> greenBlockers;
        if (Group* group = bot->GetGroup())
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member))
                    continue;

                bool hasExhaustion = member->HasAura(SPELL_NETHER_EXHAUSTION_GREEN);
                bool isRogue = member->getClass() == CLASS_ROGUE;
                bool isDpsWarrior = member->getClass() == CLASS_WARRIOR && botAI->IsDps(member);
                bool eligibleRogueWarrior = (isRogue || isDpsWarrior) && !hasExhaustion;

                if (eligibleRogueWarrior)
                    greenBlockers.push_back(member);
            }

            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (!member || !member->IsAlive() || !GET_PLAYERBOT_AI(member))
                    continue;

                bool hasExhaustion = member->HasAura(SPELL_NETHER_EXHAUSTION_GREEN);
                Aura* greenBuff = member->GetAura(SPELL_GREEN_BEAM_DEBUFF);
                bool overStack = greenBuff && greenBuff->GetStackAmount() >= 24;
                bool isHealer = botAI->IsHeal(member);
                bool eligibleHealer = isHealer && !hasExhaustion && !overStack;

                if (eligibleHealer)
                    greenBlockers.push_back(member);
            }
        }

        return greenBlockers;
    }

    std::tuple<Player*, Player*, Player*> GetCurrentBeamBlockers(PlayerbotAI* botAI, Player* bot)
    {
        static ObjectGuid currentRedBlocker;
        static ObjectGuid currentGreenBlocker;
        static ObjectGuid currentBlueBlocker;

        Player* redBlocker = nullptr;
        Player* greenBlocker = nullptr;
        Player* blueBlocker = nullptr;

        std::vector<Player*> redBlockers = GetRedBlockers(botAI, bot);
        if (!redBlockers.empty())
        {
            auto it = std::find_if(redBlockers.begin(), redBlockers.end(), [](Player* player)
            {
                return player && player->GetGUID() == currentRedBlocker;
            });

            if (it != redBlockers.end())
                redBlocker = *it;
            else
                redBlocker = redBlockers.front();

            currentRedBlocker = redBlocker ? redBlocker->GetGUID() : ObjectGuid::Empty;
        }
        else
        {
            currentRedBlocker = ObjectGuid::Empty;
            redBlocker = nullptr;
        }

        std::vector<Player*> greenBlockers = GetGreenBlockers(botAI, bot);
        if (!greenBlockers.empty())
        {
            auto it = std::find_if(greenBlockers.begin(), greenBlockers.end(), [](Player* player)
            {
                return player && player->GetGUID() == currentGreenBlocker;
            });

            if (it != greenBlockers.end())
                greenBlocker = *it;
            else
                greenBlocker = greenBlockers.front();

            currentGreenBlocker = greenBlocker ? greenBlocker->GetGUID() : ObjectGuid::Empty;
        }
        else
        {
            currentGreenBlocker = ObjectGuid::Empty;
            greenBlocker = nullptr;
        }

        std::vector<Player*> blueBlockers = GetBlueBlockers(botAI, bot);
        if (!blueBlockers.empty())
        {
            auto it = std::find_if(blueBlockers.begin(), blueBlockers.end(), [](Player* player)
            {
                return player && player->GetGUID() == currentBlueBlocker;
            });

            if (it != blueBlockers.end())
                blueBlocker = *it;
            else
                blueBlocker = blueBlockers.front();

            currentBlueBlocker = blueBlocker ? blueBlocker->GetGUID() : ObjectGuid::Empty;
        }
        else
        {
            currentBlueBlocker = ObjectGuid::Empty;
            blueBlocker = nullptr;
        }

        return std::make_tuple(redBlocker, greenBlocker, blueBlocker);
    }

    std::vector<Unit*> GetAllVoidZones(PlayerbotAI* botAI, Player* bot)
    {
        std::vector<Unit*> voidZones;
        const float radius = 30.0f;
        const GuidVector npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();
        for (auto const& npcGuid : npcs)
        {
            Unit* unit = botAI->GetUnit(npcGuid);
            if (!unit || unit->GetEntry() != NPC_VOID_ZONE)
                continue;

            float dist = bot->GetExactDist2d(unit);
            if (dist < radius)
                voidZones.push_back(unit);
        }

        return voidZones;
    }

    bool IsSafePosition(float x, float y, const std::vector<Unit*>& hazards, float hazardRadius)
    {
        for (Unit* hazard : hazards)
        {
            float dist = hazard->GetExactDist2d(x, y);
            if (dist < hazardRadius)
                return false;
        }

        return true;
    }

    std::vector<Unit*> GetSpawnedInfernals(PlayerbotAI* botAI)
    {
        std::vector<Unit*> infernals;
        const GuidVector npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();
        for (auto const& npcGuid : npcs)
        {
            Unit* unit = botAI->GetUnit(npcGuid);
            if (unit && unit->GetEntry() == NPC_NETHERSPITE_INFERNAL)
                infernals.push_back(unit);
        }

        return infernals;
    }

    bool IsMasterTankingNightbane(PlayerbotAI* botAI, Player* bot, Unit* nightbane)
    {
        if (!botAI || !bot || !nightbane)
            return false;

        Player* master = botAI->GetMaster();
        if (!master || master == bot || GET_PLAYERBOT_AI(master))
            return false;

        if (nightbane->GetVictim() == master)
            return true;

        return botAI->IsMainTank(master);
    }

    bool ShouldUseDynamicHumanTankMode(PlayerbotAI* botAI, Player* bot, Unit* nightbane)
    {
        if (!botAI || !bot || !nightbane)
            return false;

        if (bot->GetMapId() != KARAZHAN_MAP_ID || nightbane->GetPositionZ() > NIGHTBANE_FLIGHT_Z)
            return false;

        return IsMasterTankingNightbane(botAI, bot, nightbane);
    }

    Position GetNightbaneDynamicAnchorForBot(PlayerbotAI* botAI, Player* bot, Unit* nightbane)
    {
        if (!botAI || !bot || !nightbane)
            return Position(bot ? bot->GetPositionX() : 0.0f, bot ? bot->GetPositionY() : 0.0f, bot ? bot->GetPositionZ() : 0.0f);

        const float bossX = nightbane->GetPositionX();
        const float bossY = nightbane->GetPositionY();
        const float bossZ = nightbane->GetPositionZ();
        const float facing = nightbane->GetOrientation();

        float radius = 8.0f;
        static const float meleeOffsets[4] = { -0.45f, -0.20f, 0.20f, 0.45f };
        static const float healerOffsets[5] = { -1.2f, -0.6f, 0.0f, 0.6f, 1.2f };
        static const float rangedOffsets[6] = { -1.6f, -1.0f, -0.4f, 0.4f, 1.0f, 1.6f };

        uint32 slot = bot->GetGUID().GetCounter();
        float offset = 0.0f;

        if (botAI->IsMainTank(bot))
        {
            radius = 5.0f;
            offset = 0.0f;
        }
        else if (botAI->IsTank(bot))
        {
            radius = 9.0f;
            offset = meleeOffsets[slot % 4];
        }
        else if (botAI->IsHeal(bot))
        {
            radius = 20.0f;
            offset = healerOffsets[slot % 5];
        }
        else if (botAI->IsRanged(bot))
        {
            radius = 24.0f;
            offset = rangedOffsets[slot % 6];
        }
        else
        {
            radius = 6.0f;
            offset = meleeOffsets[slot % 4];
        }

        // Build anchors from the boss's rear arc to keep bots out of breath/cleave.
        const float angle = facing + static_cast<float>(M_PI) + offset;
        const float x = bossX + std::cos(angle) * radius;
        const float y = bossY + std::sin(angle) * radius;
        return Position(x, y, bossZ);
    }

    bool IsAtNightbaneDynamicAnchor(Player* bot, Position const& anchor, float tolerance)
    {
        if (!bot)
            return false;

        return bot->GetExactDist2d(anchor.GetPositionX(), anchor.GetPositionY()) <= tolerance;
    }

    bool IsSameFloorOrReasonableZ(Player* bot, Position const& anchor)
    {
        if (!bot || !bot->GetMap())
            return false;

        const float anchorZ = anchor.GetPositionZ();
        if (anchorZ <= INVALID_HEIGHT)
            return false;

        const float zDiff = std::fabs(anchorZ - bot->GetPositionZ());
        return zDiff <= 8.0f;
    }

    bool HasReasonablePathToNightbaneAnchor(Player* bot, Position const& anchor)
    {
        if (!bot || !bot->GetMap())
            return false;

        PathGenerator path(bot);
        path.CalculatePath(anchor.GetPositionX(), anchor.GetPositionY(), anchor.GetPositionZ(), false);

        PathType type = path.GetPathType();
        int typeOk = PATHFIND_NORMAL | PATHFIND_INCOMPLETE | PATHFIND_FARFROMPOLY;
        if (type & (~typeOk))
            return false;

        const float direct = bot->GetExactDist2d(anchor.GetPositionX(), anchor.GetPositionY());
        const float pathLen = path.getPathLength();
        if (direct > 3.0f && pathLen > (direct * 2.6f + 6.0f))
            return false;

        return true;
    }

    bool IsNightbaneAnchorPathSafe(Player* bot, Position const& anchor)
    {
        if (!bot || !bot->GetMap() || bot->GetMapId() != KARAZHAN_MAP_ID)
            return false;

        if (!IsSameFloorOrReasonableZ(bot, anchor))
            return false;

        float x = anchor.GetPositionX();
        float y = anchor.GetPositionY();
        float z = anchor.GetPositionZ();
        if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
                                                             x, y, z, true))
            return false;

        if (!bot->IsWithinLOS(x, y, z))
            return false;

        Position corrected(x, y, z);
        return HasReasonablePathToNightbaneAnchor(bot, corrected) && IsNightbaneMovementAllowed(bot, corrected);
    }

    bool FindNearestSafeNightbaneAnchor(Player* bot, Unit* boss, Position wanted, Position& safeOut)
    {
        if (!bot || !boss || !bot->GetMap() || boss->GetMapId() != bot->GetMapId())
            return false;

        auto tryCandidate = [&](Position candidate, float& bestDist, bool& found) -> void
        {
            float cx = candidate.GetPositionX();
            float cy = candidate.GetPositionY();
            float cz = candidate.GetPositionZ();
            if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
                                                                 cx, cy, cz, true))
                return;

            Position corrected(cx, cy, cz);
            if (!IsNightbaneAnchorPathSafe(bot, corrected))
                return;

            float d = bot->GetExactDist2d(cx, cy);
            if (!found || d < bestDist)
            {
                found = true;
                bestDist = d;
                safeOut = corrected;
            }
        };

        float bestDist = 0.0f;
        bool found = false;
        tryCandidate(wanted, bestDist, found);
        if (found)
            return true;

        const float baseRadius = std::max(5.0f, boss->GetExactDist2d(wanted.GetPositionX(), wanted.GetPositionY()));
        const float facing = boss->GetOrientation();
        static const float angleOffsets[] = { 0.0f, 0.35f, -0.35f, 0.7f, -0.7f, 1.0f, -1.0f, 1.35f, -1.35f };
        static const float radii[] = { 0.0f, 2.0f, -2.0f, 4.0f };

        for (float rOffset : radii)
        {
            float radius = std::max(4.0f, baseRadius + rOffset);
            for (float aOffset : angleOffsets)
            {
                float angle = facing + static_cast<float>(M_PI) + aOffset;
                Position candidate(boss->GetPositionX() + std::cos(angle) * radius,
                                   boss->GetPositionY() + std::sin(angle) * radius,
                                   boss->GetPositionZ());
                tryCandidate(candidate, bestDist, found);
            }
        }

        return found;
    }

    bool IsInsideNightbaneFightArea(Position const& pos)
    {
        // Nightbane Master's Terrace containment bounds.
        return pos.GetPositionX() >= -11225.0f && pos.GetPositionX() <= -11100.0f &&
               pos.GetPositionY() >= -2008.0f && pos.GetPositionY() <= -1860.0f &&
               pos.GetPositionZ() >= 86.0f && pos.GetPositionZ() <= 105.5f;
    }

    bool IsNightbanePathContained(Player* bot, Position const& dest)
    {
        if (!bot || !bot->GetMap())
            return false;

        PathGenerator path(bot);
        path.CalculatePath(dest.GetPositionX(), dest.GetPositionY(), dest.GetPositionZ(), false);
        PathType type = path.GetPathType();
        if (!(type & PATHFIND_NORMAL))
            return false;

        Movement::PointsArray points = path.GetPath();
        if (points.empty())
            return false;

        for (auto const& p : points)
        {
            if (!IsInsideNightbaneFightArea(Position(p.x, p.y, p.z)))
                return false;
        }

        return true;
    }

    bool IsNightbaneMovementAllowed(Player* bot, Position const& dest)
    {
        if (!bot || !bot->GetMap() || bot->GetMapId() != KARAZHAN_MAP_ID)
            return false;

        if (!IsInsideNightbaneFightArea(dest))
            return false;

        if (!IsSameFloorOrReasonableZ(bot, dest))
            return false;

        float x = dest.GetPositionX();
        float y = dest.GetPositionY();
        float z = dest.GetPositionZ();
        if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
                                                             x, y, z, true))
            return false;

        if (!bot->IsWithinLOS(x, y, z))
            return false;

        Position corrected(x, y, z);
        return HasReasonablePathToNightbaneAnchor(bot, corrected) && IsNightbanePathContained(bot, corrected);
    }

    bool IsNightbaneTargetAllowed(Unit* target)
    {
        if (!target)
            return false;

        return IsInsideNightbaneFightArea(Position(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ()));
    }

    bool IsInsideChessFightArea(Position const& pos)
    {
        return pos.GetPositionX() >= -11132.0f && pos.GetPositionX() <= -11035.0f &&
               pos.GetPositionY() >= -1930.0f && pos.GetPositionY() <= -1848.0f &&
               pos.GetPositionZ() >= 214.0f && pos.GetPositionZ() <= 236.0f;
    }

    bool IsChessEncounterRelevant(PlayerbotAI* botAI, Player* bot)
    {
        if (!botAI || !bot || bot->GetMapId() != KARAZHAN_MAP_ID)
            return false;

        if (IsInsideChessFightArea(Position(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ())))
            return true;

        Creature* charm = bot->GetCharm() ? bot->GetCharm()->ToCreature() : nullptr;
        return charm && IsChessPieceEntry(charm->GetEntry());
    }

    Position GetNearestNightbaneSafePoint(Player* bot)
    {
        Position fallback = NIGHTBANE_FINAL_BOSS_POSITION;
        if (!bot || !bot->GetMap())
            return fallback;

        if (IsInsideNightbaneFightArea(Position(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ())))
            return Position(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());

        Position best = fallback;
        float bestDist = std::numeric_limits<float>::max();
        const Position center = NIGHTBANE_FINAL_BOSS_POSITION;
        static const float radii[] = { 0.0f, 5.0f, 9.0f, 13.0f, 17.0f };
        static const uint8 numAngles = 24;

        for (float r : radii)
        {
            for (uint8 i = 0; i < numAngles; ++i)
            {
                float angle = (2.0f * static_cast<float>(M_PI) * i) / numAngles;
                Position cand(center.GetPositionX() + std::cos(angle) * r,
                              center.GetPositionY() + std::sin(angle) * r,
                              center.GetPositionZ());

                if (!IsInsideNightbaneFightArea(cand))
                    continue;
                if (!IsNightbaneMovementAllowed(bot, cand))
                    continue;

                float d = bot->GetExactDist2d(cand.GetPositionX(), cand.GetPositionY());
                if (d < bestDist)
                {
                    bestDist = d;
                    best = cand;
                }
            }
        }

        return best;
    }

    bool IsChessPieceEntry(uint32 entry)
    {
        switch (entry)
        {
            case NPC_PAWN_H:
            case NPC_PAWN_A:
            case NPC_KNIGHT_H:
            case NPC_KNIGHT_A:
            case NPC_QUEEN_H:
            case NPC_QUEEN_A:
            case NPC_BISHOP_H:
            case NPC_BISHOP_A:
            case NPC_ROOK_H:
            case NPC_ROOK_A:
            case NPC_KING_H:
            case NPC_KING_A:
                return true;
            default:
                return false;
        }
    }

    ChessSide GetChessSideForBot(Player* bot)
    {
        if (!bot)
            return ChessSide::UNKNOWN;

        Player* owner = bot;
        if (PlayerbotAI* ai = GET_PLAYERBOT_AI(bot))
        {
            if (Player* master = ai->GetMaster())
                owner = master;
        }

        if (owner->GetTeamId(true) == TEAM_ALLIANCE)
            return ChessSide::ALLIANCE;
        if (owner->GetTeamId(true) == TEAM_HORDE)
            return ChessSide::HORDE;
        return ChessSide::UNKNOWN;
    }

    bool IsFriendlyChessPieceForBot(Player* bot, Creature* piece)
    {
        if (!bot || !piece || !IsChessPieceEntry(piece->GetEntry()))
            return false;
        if (!piece->IsInWorld())
            return false;

        ChessSide side = GetChessSideForBot(bot);
        if (side == ChessSide::ALLIANCE)
        {
            return piece->GetEntry() == NPC_PAWN_A || piece->GetEntry() == NPC_KNIGHT_A ||
                   piece->GetEntry() == NPC_QUEEN_A || piece->GetEntry() == NPC_BISHOP_A ||
                   piece->GetEntry() == NPC_ROOK_A || piece->GetEntry() == NPC_KING_A;
        }
        if (side == ChessSide::HORDE)
        {
            return piece->GetEntry() == NPC_PAWN_H || piece->GetEntry() == NPC_KNIGHT_H ||
                   piece->GetEntry() == NPC_QUEEN_H || piece->GetEntry() == NPC_BISHOP_H ||
                   piece->GetEntry() == NPC_ROOK_H || piece->GetEntry() == NPC_KING_H;
        }

        return false;
    }

    bool IsEnemyChessPieceForBot(Player* bot, Creature* piece)
    {
        if (!bot || !piece || !IsChessPieceEntry(piece->GetEntry()))
            return false;
        if (!piece->IsInWorld())
            return false;

        ChessSide side = GetChessSideForBot(bot);
        if (side == ChessSide::ALLIANCE)
        {
            return piece->GetEntry() == NPC_PAWN_H || piece->GetEntry() == NPC_KNIGHT_H ||
                   piece->GetEntry() == NPC_QUEEN_H || piece->GetEntry() == NPC_BISHOP_H ||
                   piece->GetEntry() == NPC_ROOK_H || piece->GetEntry() == NPC_KING_H;
        }
        if (side == ChessSide::HORDE)
        {
            return piece->GetEntry() == NPC_PAWN_A || piece->GetEntry() == NPC_KNIGHT_A ||
                   piece->GetEntry() == NPC_QUEEN_A || piece->GetEntry() == NPC_BISHOP_A ||
                   piece->GetEntry() == NPC_ROOK_A || piece->GetEntry() == NPC_KING_A;
        }

        return false;
    }

    bool IsClaimableChessPieceForBot(Player* bot, Creature* piece, bool allowControlledState /*= false*/)
    {
        if (!bot || !piece)
            return false;

        if (GetChessSideForBot(bot) == ChessSide::UNKNOWN)
            return false;

        if (!IsFriendlyChessPieceForBot(bot, piece))
            return false;

        if (!piece->IsInWorld())
            return false;

        if (!piece->IsAlive())
            return false;

        if (allowControlledState)
            return true;

        return !piece->IsCharmed() &&
               !piece->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
    }

    bool IsClaimableFriendlyPawnForOpening(Player* bot, Creature* piece)
    {
        if (!bot || !piece)
            return false;

        uint32 const entry = piece->GetEntry();
        ChessSide const side = GetChessSideForBot(bot);
        if (side == ChessSide::UNKNOWN)
            return false;

        // Opening claim should not depend on pre-control select/charm state.
        // Chess pieces may already be charmed by script-side controllers but are still valid
        // targets for SPELL_CONTROL_PIECE takeover.
        if (!piece->IsInWorld() || !piece->IsAlive())
            return false;

        if (!IsFriendlyChessPieceForBot(bot, piece))
            return false;

        if (side == ChessSide::HORDE)
            return entry == NPC_PAWN_H;
        if (side == ChessSide::ALLIANCE)
            return entry == NPC_PAWN_A;

        return false;
    }

    bool IsHealerChessPieceEntry(uint32 entry)
    {
        return entry == NPC_BISHOP_A || entry == NPC_BISHOP_H;
    }

    bool IsKingChessPieceEntry(uint32 entry)
    {
        return entry == NPC_KING_A || entry == NPC_KING_H;
    }

    bool IsDamageChessPieceEntry(uint32 entry)
    {
        return entry == NPC_QUEEN_A || entry == NPC_QUEEN_H || entry == NPC_ROOK_A || entry == NPC_ROOK_H;
    }

    bool IsChessPhaseInProgress(Player* bot)
    {
        if (!bot)
            return false;

        InstanceMap* map = bot->GetMap() ? bot->GetMap()->ToInstanceMap() : nullptr;
        InstanceScript* instance = map ? map->GetInstanceScript() : nullptr;
        if (!instance)
            return false;

        uint32 phase = instance->GetData(DATA_CHESS_GAME_PHASE);
        return phase == CHESS_PHASE_INPROGRESS_PVE || phase == CHESS_PHASE_INPROGRESS_PVP;
    }

    void CountEnemyChessBoardState(PlayerbotAI* botAI, Player* bot, uint32& supportAlive, uint32& damageAlive, uint32& pawnAlive)
    {
        supportAlive = 0;
        damageAlive = 0;
        pawnAlive = 0;

        if (!botAI || !bot)
            return;

        for (Creature* piece : GetNearbyChessPieces(botAI, bot, false))
        {
            if (!piece || !piece->IsAlive() || !IsEnemyChessPieceForBot(bot, piece) || IsKingChessPieceEntry(piece->GetEntry()))
                continue;

            switch (piece->GetEntry())
            {
                case NPC_BISHOP_A:
                case NPC_BISHOP_H:
                    ++supportAlive;
                    break;
                case NPC_ROOK_A:
                case NPC_ROOK_H:
                case NPC_QUEEN_A:
                case NPC_QUEEN_H:
                case NPC_KNIGHT_A:
                case NPC_KNIGHT_H:
                    ++damageAlive;
                    break;
                case NPC_PAWN_A:
                case NPC_PAWN_H:
                    ++pawnAlive;
                    break;
                default:
                    ++damageAlive;
                    break;
            }
        }
    }

    bool IsOnActiveChessBoard(Creature* piece)
    {
        if (!piece || !piece->IsInWorld() || !piece->IsAlive() || !IsChessPieceEntry(piece->GetEntry()))
            return false;

        // Same Karazhan chess board projection used by the action-side active-board checks.
        constexpr float originX = -11108.099609f;
        constexpr float originY = -1872.910034f;
        constexpr float rowStepX = 4.4f;
        constexpr float rowStepY = 3.45f;
        constexpr float colStepX = 3.49f;
        constexpr float colStepY = -4.4f;
        constexpr float determinant = rowStepX * colStepY - rowStepY * colStepX;
        if (std::fabs(determinant) < 0.001f)
            return false;

        float const dx = piece->GetPositionX() - originX;
        float const dy = piece->GetPositionY() - originY;
        float const rowF = (dx * colStepY - dy * colStepX) / determinant;
        float const colF = (rowStepX * dy - rowStepY * dx) / determinant;
        int const row = static_cast<int>(std::lround(rowF));
        int const col = static_cast<int>(std::lround(colF));

        if (row < 0 || row > 7 || col < 0 || col > 7)
            return false;

        float const expectedX = originX + row * rowStepX + col * colStepX;
        float const expectedY = originY + row * rowStepY + col * colStepY;
        float const distSq = (piece->GetPositionX() - expectedX) * (piece->GetPositionX() - expectedX) +
            (piece->GetPositionY() - expectedY) * (piece->GetPositionY() - expectedY);
        return distSq <= 10.24f;
    }

    void CountActiveBoardEnemyChessBoardState(PlayerbotAI* botAI, Player* bot, uint32& supportAlive, uint32& damageAlive, uint32& pawnAlive, uint32& activeNonKingRemaining)
    {
        supportAlive = 0;
        damageAlive = 0;
        pawnAlive = 0;
        activeNonKingRemaining = 0;

        if (!botAI || !bot)
            return;

        for (Creature* piece : GetNearbyChessPieces(botAI, bot, false))
        {
            if (!piece || !IsEnemyChessPieceForBot(bot, piece) || IsKingChessPieceEntry(piece->GetEntry()))
                continue;

            if (!IsOnActiveChessBoard(piece))
                continue;

            ++activeNonKingRemaining;
            switch (piece->GetEntry())
            {
                case NPC_BISHOP_A:
                case NPC_BISHOP_H:
                    ++supportAlive;
                    break;
                case NPC_ROOK_A:
                case NPC_ROOK_H:
                case NPC_QUEEN_A:
                case NPC_QUEEN_H:
                case NPC_KNIGHT_A:
                case NPC_KNIGHT_H:
                    ++damageAlive;
                    break;
                case NPC_PAWN_A:
                case NPC_PAWN_H:
                    ++pawnAlive;
                    break;
                default:
                    ++damageAlive;
                    break;
            }
        }
    }

    bool IsKarazhanChessKingFocusAllowed(PlayerbotAI* botAI, Player* bot, Creature* enemyKing, uint32& supportAlive, uint32& damageAlive, uint32& pawnAlive, std::string& gateReason)
    {
        CountEnemyChessBoardState(botAI, bot, supportAlive, damageAlive, pawnAlive);

        bool const kingCriticallyLow = enemyKing && enemyKing->GetHealthPct() < 30.0f;
        bool const boardMostlyCleared = supportAlive == 0 && damageAlive == 0 && pawnAlive <= 3;
        gateReason = kingCriticallyLow ? "king_critically_low" : (boardMostlyCleared ? "board_mostly_cleared" : "board_not_cleared");
        return kingCriticallyLow || boardMostlyCleared;
    }

    bool IsKarazhanChessKingFocusAllowedActiveBoard(PlayerbotAI* botAI, Player* bot, Creature* enemyKing, uint32& supportAlive, uint32& damageAlive, uint32& pawnAlive, uint32& activeNonKingRemaining, std::string& gateReason)
    {
        CountActiveBoardEnemyChessBoardState(botAI, bot, supportAlive, damageAlive, pawnAlive, activeNonKingRemaining);

        bool const kingCriticallyLow = enemyKing && enemyKing->GetHealthPct() < 30.0f;
        bool const boardMostlyCleared = supportAlive == 0 && damageAlive == 0 && pawnAlive <= 3;
        gateReason = kingCriticallyLow ? "king_critically_low" : (boardMostlyCleared ? "board_mostly_cleared" : "board_not_cleared");
        return kingCriticallyLow || boardMostlyCleared;
    }

    bool IsChessEventActive(PlayerbotAI* botAI, Player* bot)
    {
        if (!botAI || !bot || bot->GetMapId() != KARAZHAN_MAP_ID || !IsChessEncounterRelevant(botAI, bot))
            return false;

        size_t scannedCount = 0;
        bool usedCache = false;
        ChessNearbyCache const& cache = GetChessNearbyCache(botAI, bot, scannedCount, usedCache);
        (void)scannedCount;

        if (cache.hasMedivhHazardNearby)
            return true;

        if (HasChessSessionAura(bot->GetGroup()))
            return true;

        return IsChessPhaseInProgress(bot) || (cache.hasChessPieceNearby && cache.hasCombatPieceNearby);
    }

    Creature* GetAssignedChessPiece(Player* bot)
    {
        if (!bot)
            return nullptr;

        auto it = chessAssignedPieceByBot.find(bot->GetGUID());
        if (it == chessAssignedPieceByBot.end())
            return nullptr;

        Unit* unit = ObjectAccessor::GetUnit(*bot, it->second);
        Creature* piece = unit ? unit->ToCreature() : nullptr;
        if (!piece || !piece->IsInWorld() || !piece->IsAlive() || !IsChessPieceEntry(piece->GetEntry()))
            return nullptr;

        return piece;
    }

    bool SetAssignedChessPiece(Player* bot, Creature* piece, time_t lockSeconds)
    {
        if (!bot || !piece)
            return false;

        if (IsPieceAssignedToOtherBot(bot, piece))
            return false;

        ClearAssignedChessPiece(bot);
        chessAssignedPieceByBot[bot->GetGUID()] = piece->GetGUID();
        chessAssignedBotByPiece[piece->GetGUID()] = bot->GetGUID();
        chessAssignmentLockUntil[bot->GetGUID()] = std::time(nullptr) + lockSeconds;
        return true;
    }

    void ClearAssignedChessPiece(Player* bot)
    {
        if (!bot)
            return;

        auto it = chessAssignedPieceByBot.find(bot->GetGUID());
        if (it != chessAssignedPieceByBot.end())
        {
            chessAssignedBotByPiece.erase(it->second);
            chessAssignedPieceByBot.erase(it);
        }
        chessAssignmentLockUntil.erase(bot->GetGUID());
    }

    bool IsPieceAssignedToOtherBot(Player* bot, Creature* piece)
    {
        if (!bot || !piece)
            return false;

        auto it = chessAssignedBotByPiece.find(piece->GetGUID());
        return it != chessAssignedBotByPiece.end() && it->second != bot->GetGUID();
    }

    std::vector<Creature*> GetNearbyChessPieces(PlayerbotAI* botAI, Player* bot, bool friendlyOnly)
    {
        std::vector<Creature*> pieces;
        size_t scannedCount = 0;
        bool usedCache = false;
        ChessNearbyCache const& cache = GetChessNearbyCache(botAI, bot, scannedCount, usedCache);
        if (!botAI || !bot || bot->GetMapId() != KARAZHAN_MAP_ID)
            return pieces;

        if (usedCache)
            return friendlyOnly ? cache.friendlyPieces : cache.pieces;

        pieces = friendlyOnly ? cache.friendlyPieces : cache.pieces;
        return pieces;
    }

    Creature* GetFriendlyChessKing(PlayerbotAI* botAI, Player* bot)
    {
        for (Creature* piece : GetNearbyChessPieces(botAI, bot, true))
        {
            if (IsKingChessPieceEntry(piece->GetEntry()) && IsFriendlyChessPieceForBot(bot, piece))
                return piece;
        }
        return nullptr;
    }

    Creature* GetEnemyChessKing(PlayerbotAI* botAI, Player* bot)
    {
        if (!botAI || !bot)
            return nullptr;
        for (Creature* piece : GetNearbyChessPieces(botAI, bot, false))
            if (piece && piece->IsAlive() && IsKingChessPieceEntry(piece->GetEntry()) && IsEnemyChessPieceForBot(bot, piece))
                return piece;
        return nullptr;
    }

    std::vector<Creature*> GetNearbyChessMoveTriggers(PlayerbotAI* botAI, Player* bot)
    {
        std::vector<Creature*> triggers;
        size_t scannedCount = 0;
        bool usedCache = false;
        ChessNearbyCache const& cache = GetChessNearbyCache(botAI, bot, scannedCount, usedCache);
        if (!botAI || !bot || bot->GetMapId() != KARAZHAN_MAP_ID)
            return triggers;

        if (usedCache)
            return cache.moveTriggers;

        triggers = cache.moveTriggers;
        return triggers;
    }

    bool IsStraightPathSafe(const Position& start, const Position& target, const std::vector<Unit*>& hazards,
                            float hazardRadius, float stepSize)
    {
        float sx = start.GetPositionX();
        float sy = start.GetPositionY();
        float tx = target.GetPositionX();
        float ty = target.GetPositionY();

        const float totalDist = start.GetExactDist2d(target.GetPositionX(), target.GetPositionY());
        if (totalDist == 0.0f)
            return true;

        for (float checkDist = 0.0f; checkDist <= totalDist; checkDist += stepSize)
        {
            float t = checkDist / totalDist;
            float checkX = sx + (tx - sx) * t;
            float checkY = sy + (ty - sy) * t;
            for (Unit* hazard : hazards)
            {
                const float hx = checkX - hazard->GetPositionX();
                const float hy = checkY - hazard->GetPositionY();
                if ((hx*hx + hy*hy) < hazardRadius * hazardRadius)
                    return false;
            }
        }

        return true;
    }

    bool TryFindSafePositionWithSafePath(
        Player* bot, float originX, float originY, float originZ, float centerX, float centerY, float centerZ,
        const std::vector<Unit*>& hazards, float safeDistance, float stepSize, uint8 numAngles,
        float maxSampleDist, bool requireSafePath, float& bestDestX, float& bestDestY, float& bestDestZ)
    {
        float bestMoveDist = std::numeric_limits<float>::max();
        bool found = false;

        for (int i = 0; i < numAngles; ++i)
        {
            float angle = (2.0f * M_PI * i) / numAngles;
            float dx = cos(angle);
            float dy = sin(angle);

            for (float dist = stepSize; dist <= maxSampleDist; dist += stepSize)
            {
                float x = centerX + dx * dist;
                float y = centerY + dy * dist;
                float z = centerZ;
                float destX = x, destY = y, destZ = z;
                if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, centerX, centerY, centerZ,
                                                                    destX, destY, destZ, true))
                    continue;

                if (!IsSafePosition(destX, destY, hazards, safeDistance))
                    continue;

                if (requireSafePath)
                {
                    if (!IsStraightPathSafe(Position(originX, originY, originZ), Position(destX, destY, destZ),
                                            hazards, safeDistance, stepSize))
                        continue;
                }

                const float moveDist = Position(originX, originY, originZ).GetExactDist2d(destX, destY);
                if (moveDist < bestMoveDist)
                {
                    bestMoveDist = moveDist;
                    bestDestX = destX;
                    bestDestY = destY;
                    bestDestZ = destZ;
                    found = true;
                }
            }
        }

        return found;
    }
}
