/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_KARACHESSHELPERS_H
#define PLAYERBOTS_KARACHESSHELPERS_H

#include "AiObject.h"
#include "Position.h"
#include "Unit.h"
#include <ctime>
#include <string>
#include <unordered_map>
#include <vector>

namespace KarazhanHelpers
{
    enum KarazhanChessSpells
    {
        SPELL_GAME_IN_SESSION = 39331,
        SPELL_CONTROL_PIECE = 30019,
        SPELL_MOVE_GENERIC = 30012,
        SPELL_MOVE_COOLDOWN = 30543,
    };

    enum KarazhanChessNpcs
    {
        NPC_ECHO_OF_MEDIVH = 16816,
        NPC_CHESS_EVENT_MEDIVH_CHEAT_FIRES = 22521,
        NPC_PAWN_H = 17469,
        NPC_PAWN_A = 17211,
        NPC_KNIGHT_H = 21748,
        NPC_KNIGHT_A = 21664,
        NPC_QUEEN_H = 21750,
        NPC_QUEEN_A = 21683,
        NPC_BISHOP_H = 21747,
        NPC_BISHOP_A = 21682,
        NPC_ROOK_H = 21726,
        NPC_ROOK_A = 21160,
        NPC_KING_H = 21752,
        NPC_KING_A = 21684,
    };

    constexpr uint32 KARAZHAN_MAP_ID = 532;
    constexpr uint32 DATA_CHESS_GAME_PHASE = 35;
    constexpr uint32 CHESS_PHASE_INPROGRESS_PVE = 2;
    constexpr uint32 CHESS_PHASE_INPROGRESS_PVP = 6;

    enum class ChessSide : uint8
    {
        UNKNOWN = 0,
        ALLIANCE = 1,
        HORDE = 2,
    };

    extern std::unordered_map<ObjectGuid, ObjectGuid> chessAssignedPieceByBot;
    extern std::unordered_map<ObjectGuid, ObjectGuid> chessAssignedBotByPiece;
    extern std::unordered_map<ObjectGuid, time_t> chessAssignmentLockUntil;
    extern std::unordered_map<ObjectGuid, time_t> chessSelfAbilityThrottleByPiece;

    bool IsInsideChessFightArea(Position const& pos);
    bool IsChessEncounterRelevant(PlayerbotAI* botAI, Player* bot);
    bool IsChessPieceEntry(uint32 entry);
    ChessSide GetChessSideForBot(Player* bot);
    bool IsFriendlyChessPieceForBot(Player* bot, Creature* piece);
    bool IsEnemyChessPieceForBot(Player* bot, Creature* piece);
    bool IsClaimableChessPieceForBot(Player* bot, Creature* piece, bool allowControlledState = false);
    bool IsClaimableFriendlyPawnForOpening(Player* bot, Creature* piece);
    bool IsHealerChessPieceEntry(uint32 entry);
    bool IsKingChessPieceEntry(uint32 entry);
    bool IsDamageChessPieceEntry(uint32 entry);
    bool IsChessEventActive(PlayerbotAI* botAI, Player* bot);
    bool IsChessPhaseInProgress(Player* bot);
    bool IsOnActiveChessBoard(Creature* piece);
    void CountEnemyChessBoardState(PlayerbotAI* botAI, Player* bot, uint32& supportAlive, uint32& damageAlive,
        uint32& pawnAlive);
    void CountActiveBoardEnemyChessBoardState(PlayerbotAI* botAI, Player* bot, uint32& supportAlive,
        uint32& damageAlive, uint32& pawnAlive, uint32& activeNonKingRemaining);
    bool IsKarazhanChessKingFocusAllowed(PlayerbotAI* botAI, Player* bot, Creature* enemyKing,
        uint32& supportAlive, uint32& damageAlive, uint32& pawnAlive, std::string& gateReason);
    bool IsKarazhanChessKingFocusAllowedActiveBoard(PlayerbotAI* botAI, Player* bot, Creature* enemyKing,
        uint32& supportAlive, uint32& damageAlive, uint32& pawnAlive, uint32& activeNonKingRemaining,
        std::string& gateReason);
    Creature* GetAssignedChessPiece(Player* bot);
    bool SetAssignedChessPiece(Player* bot, Creature* piece, time_t lockSeconds = 8);
    void ClearAssignedChessPiece(Player* bot);
    bool IsPieceAssignedToOtherBot(Player* bot, Creature* piece);
    std::vector<Creature*> GetNearbyChessPieces(PlayerbotAI* botAI, Player* bot, bool friendlyOnly);
    Creature* GetFriendlyChessKing(PlayerbotAI* botAI, Player* bot);
    Creature* GetEnemyChessKing(PlayerbotAI* botAI, Player* bot);
    std::vector<Creature*> GetNearbyChessMoveTriggers(PlayerbotAI* botAI, Player* bot);
    bool IsStraightPathSafe(Position const& start, Position const& target, std::vector<Unit*> const& hazards,
        float hazardRadius, float stepSize);
    bool TryFindSafePositionWithSafePath(Player* bot, float originX, float originY, float originZ, float centerX,
        float centerY, float centerZ, std::vector<Unit*> const& hazards, float safeDistance, float stepSize,
        uint8 numAngles, float maxSampleDist, bool requireSafePath, float& bestDestX, float& bestDestY,
        float& bestDestZ);
}

#endif
