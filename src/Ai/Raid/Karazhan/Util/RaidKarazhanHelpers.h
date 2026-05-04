#ifndef _PLAYERBOT_RAIDKARAZHANHELPERS_H_
#define _PLAYERBOT_RAIDKARAZHANHELPERS_H_

#include <ctime>
#include <unordered_map>
#include <set>

#include "AiObject.h"
#include "Position.h"
#include "Unit.h"

namespace KarazhanHelpers
{
    enum KarazhanSpells
    {
        // Maiden of Virtue
        SPELL_REPENTANCE                 = 29511,

        // Opera Event
        SPELL_LITTLE_RED_RIDING_HOOD     = 30756,

        // The Curator
        SPELL_CURATOR_EVOCATION          = 30254,

        // Shade of Aran
        SPELL_FLAME_WREATH_CAST          = 30004,
        SPELL_FLAME_WREATH_AURA          = 29946,
        SPELL_ARCANE_EXPLOSION           = 29973,

        // Netherspite
        SPELL_RED_BEAM_DEBUFF            = 30421, // "Nether Portal - Perseverance" (player aura)
        SPELL_GREEN_BEAM_DEBUFF          = 30422, // "Nether Portal - Serenity" (player aura)
        SPELL_BLUE_BEAM_DEBUFF           = 30423, // "Nether Portal - Dominance" (player aura)
        SPELL_GREEN_BEAM_HEAL            = 30467, // "Nether Portal - Serenity" (Netherspite aura)
        SPELL_NETHER_EXHAUSTION_RED      = 38637,
        SPELL_NETHER_EXHAUSTION_GREEN    = 38638,
        SPELL_NETHER_EXHAUSTION_BLUE     = 38639,
        SPELL_NETHERSPITE_BANISHED       = 39833, // "Vortex Shade Black"
        SPELL_GAME_IN_SESSION            = 39331,
        SPELL_CONTROL_PIECE              = 30019,
        SPELL_MOVE_GENERIC               = 30012,
        SPELL_MOVE_COOLDOWN              = 30543,

        // Prince Malchezaar
        SPELL_ENFEEBLE                   = 30843,

        // Nightbane
        SPELL_CHARRED_EARTH              = 30129,
        SPELL_BELLOWING_ROAR             = 36922,
        SPELL_RAIN_OF_BONES              = 37091,

        // Warlock
        SPELL_WARLOCK_BANISH             = 18647,

        // Priest
        SPELL_FEAR_WARD                  =  6346,
    };

    enum KarazhanNPCs
    {
        // Trash
        NPC_SPECTRAL_RETAINER            = 16410,
        NPC_MANA_WARP                    = 16530,

        // Attumen the Huntsman
        NPC_ATTUMEN_THE_HUNTSMAN         = 15550,
        NPC_ATTUMEN_THE_HUNTSMAN_MOUNTED = 16152,

        // Terestian Illhoof
        NPC_DEMON_CHAINS                 = 17248,
        NPC_KILREK                       = 17229,

        // Shade of Aran
        NPC_CONJURED_ELEMENTAL           = 17167,

        // Netherspite
        NPC_VOID_ZONE                    = 16697,
        NPC_GREEN_PORTAL                 = 17367, // "Nether Portal - Serenity <Healing Portal>"
        NPC_BLUE_PORTAL                  = 17368, // "Nether Portal - Dominance <Damage Portal>"
        NPC_RED_PORTAL                   = 17369, // "Nether Portal - Perseverance <Tanking Portal>"
        NPC_ECHO_OF_MEDIVH               = 16816,
        NPC_CHESS_EVENT_MEDIVH_CHEAT_FIRES = 22521,
        NPC_PAWN_H                       = 17469,
        NPC_PAWN_A                       = 17211,
        NPC_KNIGHT_H                     = 21748,
        NPC_KNIGHT_A                     = 21664,
        NPC_QUEEN_H                      = 21750,
        NPC_QUEEN_A                      = 21683,
        NPC_BISHOP_H                     = 21747,
        NPC_BISHOP_A                     = 21682,
        NPC_ROOK_H                       = 21726,
        NPC_ROOK_A                       = 21160,
        NPC_KING_H                       = 21752,
        NPC_KING_A                       = 21684,

        // Prince Malchezaar
        NPC_NETHERSPITE_INFERNAL         = 17646,
    };

    constexpr uint32 KARAZHAN_MAP_ID = 532;
    constexpr float NIGHTBANE_FLIGHT_Z = 95.0f;
    constexpr uint32 DATA_CHESS_GAME_PHASE = 35;
    constexpr uint32 CHESS_PHASE_INPROGRESS_PVE = 2;
    constexpr uint32 CHESS_PHASE_INPROGRESS_PVP = 6;
    
    enum class ChessSide : uint8
    {
        UNKNOWN = 0,
        ALLIANCE = 1,
        HORDE = 2
    };

    // Attumen the Huntsman
    extern std::unordered_map<uint32, time_t> attumenDpsWaitTimer;
    // Big Bad Wolf
    extern std::unordered_map<ObjectGuid, uint8> bigBadWolfRunIndex;
    // Netherspite
    extern std::unordered_map<uint32, time_t> netherspiteDpsWaitTimer;
    extern std::unordered_map<ObjectGuid, time_t> redBeamMoveTimer;
    extern std::unordered_map<ObjectGuid, bool> lastBeamMoveSideways;
    // Nightbane
    extern std::unordered_map<uint32, time_t> nightbaneDpsWaitTimer;
    extern std::unordered_map<ObjectGuid, uint8> nightbaneTankStep;
    extern std::unordered_map<ObjectGuid, uint8> nightbaneRangedStep;
    extern std::unordered_map<uint32, time_t> nightbaneFlightPhaseStartTimer;
    extern std::unordered_map<uint32, bool> nightbaneWasInFlightPhase;
    extern std::unordered_map<ObjectGuid, bool> nightbaneRainOfBonesHit;
    extern std::unordered_map<ObjectGuid, ObjectGuid> chessAssignedPieceByBot;
    extern std::unordered_map<ObjectGuid, ObjectGuid> chessAssignedBotByPiece;
    extern std::unordered_map<ObjectGuid, time_t> chessAssignmentLockUntil;
    extern std::unordered_map<ObjectGuid, time_t> chessSelfAbilityThrottleByPiece;

    extern const Position MAIDEN_OF_VIRTUE_BOSS_POSITION;
    extern const Position MAIDEN_OF_VIRTUE_RANGED_POSITION[8];
    extern const Position BIG_BAD_WOLF_BOSS_POSITION;
    extern const Position BIG_BAD_WOLF_RUN_POSITION[4];
    extern const Position THE_CURATOR_BOSS_POSITION;
    extern const Position NIGHTBANE_TRANSITION_BOSS_POSITION;
    extern const Position NIGHTBANE_FINAL_BOSS_POSITION;
    extern const Position NIGHTBANE_RANGED_POSITION1;
    extern const Position NIGHTBANE_RANGED_POSITION2;
    extern const Position NIGHTBANE_RANGED_POSITION3;
    extern const Position NIGHTBANE_FLIGHT_STACK_POSITION;
    extern const Position NIGHTBANE_RAIN_OF_BONES_POSITION;

    Unit* GetFirstAliveUnit(const std::vector<Unit*>& units);
    bool IsFlameWreathActive(PlayerbotAI* botAI, Player* bot);
    std::vector<Player*> GetRedBlockers(PlayerbotAI* botAI, Player* bot);
    std::vector<Player*> GetBlueBlockers(PlayerbotAI* botAI, Player* bot);
    std::vector<Player*> GetGreenBlockers(PlayerbotAI* botAI, Player* bot);
    std::tuple<Player*, Player*, Player*> GetCurrentBeamBlockers(PlayerbotAI* botAI, Player* bot);
    std::vector<Unit*> GetAllVoidZones(PlayerbotAI *botAI, Player* bot);
    bool IsSafePosition (float x, float y, const std::vector<Unit*>& hazards, float hazardRadius);
    std::vector<Unit*> GetSpawnedInfernals(PlayerbotAI* botAI);
    bool IsKarazhanChessEnabled();
    bool IsKarazhanChessDebugEnabled();
    bool IsKarazhanNightbaneEnabled();
    bool IsKarazhanNightbaneDebugEnabled();
    bool IsMasterTankingNightbane(PlayerbotAI* botAI, Player* bot, Unit* nightbane);
    bool ShouldUseDynamicHumanTankMode(PlayerbotAI* botAI, Player* bot, Unit* nightbane);
    Position GetNightbaneDynamicAnchorForBot(PlayerbotAI* botAI, Player* bot, Unit* nightbane);
    bool IsAtNightbaneDynamicAnchor(Player* bot, Position const& anchor, float tolerance = 2.5f);
    bool IsSameFloorOrReasonableZ(Player* bot, Position const& anchor);
    bool HasReasonablePathToNightbaneAnchor(Player* bot, Position const& anchor);
    bool IsNightbaneAnchorPathSafe(Player* bot, Position const& anchor);
    bool FindNearestSafeNightbaneAnchor(Player* bot, Unit* boss, Position wanted, Position& safeOut);
    bool IsInsideNightbaneFightArea(Position const& pos);
    bool IsNightbanePathContained(Player* bot, Position const& dest);
    bool IsNightbaneMovementAllowed(Player* bot, Position const& dest);
    bool IsNightbaneTargetAllowed(Unit* target);
    Position GetNearestNightbaneSafePoint(Player* bot);
    bool IsChessPieceEntry(uint32 entry);
    ChessSide GetChessSideForBot(Player* bot);
    bool IsFriendlyChessPieceForBot(Player* bot, Creature* piece);
    bool IsEnemyChessPieceForBot(Player* bot, Creature* piece);
    bool IsClaimableChessPieceForBot(Player* bot, Creature* piece);
    bool IsClaimableFriendlyPawnForOpening(Player* bot, Creature* piece);
    bool IsHealerChessPieceEntry(uint32 entry);
    bool IsKingChessPieceEntry(uint32 entry);
    bool IsDamageChessPieceEntry(uint32 entry);
    bool IsChessEventActive(PlayerbotAI* botAI, Player* bot);
    bool IsChessPhaseInProgress(Player* bot);
    Creature* GetAssignedChessPiece(Player* bot);
    bool SetAssignedChessPiece(Player* bot, Creature* piece, time_t lockSeconds = 8);
    void ClearAssignedChessPiece(Player* bot);
    bool IsPieceAssignedToOtherBot(Player* bot, Creature* piece);
    std::vector<Creature*> GetNearbyChessPieces(PlayerbotAI* botAI, Player* bot, bool friendlyOnly);
    Creature* GetFriendlyChessKing(PlayerbotAI* botAI, Player* bot);
    Creature* GetEnemyChessKing(PlayerbotAI* botAI, Player* bot);
    std::vector<Creature*> GetNearbyChessMoveTriggers(PlayerbotAI* botAI, Player* bot);
    void LogKarazhanChessDebug(Player* bot, std::string const& reason);
    void LogKarazhanNightbaneDebug(Player* bot, std::string const& reason);
    bool IsStraightPathSafe(
        const Position& start, const Position& target,
        const std::vector<Unit*>& hazards, float hazardRadius, float stepSize);
    bool TryFindSafePositionWithSafePath(
        Player* bot, float originX, float originY, float originZ, float centerX, float centerY, float centerZ,
        const std::vector<Unit*>& hazards, float safeDistance, float stepSize, uint8 numAngles,
        float maxSampleDist, bool requireSafePath, float& bestDestX, float& bestDestY, float& bestDestZ);
}

#endif
