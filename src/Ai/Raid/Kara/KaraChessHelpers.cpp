/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "KaraChessHelpers.h"
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

    std::unordered_map<ObjectGuid, ObjectGuid> chessAssignedPieceByBot;
    std::unordered_map<ObjectGuid, ObjectGuid> chessAssignedBotByPiece;
    std::unordered_map<ObjectGuid, time_t> chessAssignmentLockUntil;
    std::unordered_map<ObjectGuid, time_t> chessSelfAbilityThrottleByPiece;
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

                if (!KaraHelpers::IsSafePosition(destX, destY, hazards, safeDistance))
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
