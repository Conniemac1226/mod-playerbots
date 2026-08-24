/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "KaraActions.h"
#include "KaraHelpers.h"
#include "KaraChessHelpers.h"
#include "Config.h"
#include "Group.h"
#include "Log.h"
#include "Playerbots.h"
#include "PlayerbotTextMgr.h"
#include "RaidBossHelpers.h"
#include "RtiTargetValue.h"
#include "MotionMaster.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <ctime>
#include <limits>
#include <unordered_map>
#include <unordered_set>

using namespace KarazhanHelpers;

// Chess Event
// Conservative helper mode: keep bots passive so they do not grief piece control or aggro patterns.
namespace
{
    std::string ChessSideToString(ChessSide side)
    {
        switch (side)
        {
            case ChessSide::ALLIANCE: return "ALLIANCE";
            case ChessSide::HORDE: return "HORDE";
            default: return "UNKNOWN";
        }
    }

    std::string PieceSideToString(Player* bot, Creature* piece)
    {
        if (!piece)
            return "UNKNOWN";
        if (IsFriendlyChessPieceForBot(bot, piece))
            return "FRIENDLY";
        if (IsEnemyChessPieceForBot(bot, piece))
            return "ENEMY";
        return "UNKNOWN";
    }

    struct ChessSquare
    {
        int row = -1;
        int col = -1;
    };

    struct ChessPendingMove
    {
        ChessSquare fromSquare;
        ChessSquare toSquare;
        uint32 instanceId = 0;
        time_t createdAt = 0;
        time_t expiresAt = 0;
        uint32 moveSpell = 0;
        ObjectGuid triggerGuid;
        ObjectGuid tacticalTargetGuid;
        std::string source;
    };

    struct ChessPendingClaim
    {
        ObjectGuid pieceGuid;
        uint32 instanceId = 0;
        time_t createdAt = 0;
        time_t expiresAt = 0;
        time_t nextControlAttemptAt = 0;
        uint8 controlAttempts = 0;
    };

    struct ChessAbilityNoOpBackoff
    {
        time_t until = 0;
    };

    struct ChessTargetFailureState
    {
        ObjectGuid targetGuid;
        uint8 failedMoves = 0;
        time_t blockedUntil = 0;
    };

    struct ChessDiagnosticState
    {
        std::string fingerprint;
        uint32 lastLoggedMs = 0;
    };

    std::unordered_map<ObjectGuid, ObjectGuid> chessLastFailedMoveTriggerByPiece;
    std::unordered_map<ObjectGuid, time_t> chessLastFailedMoveTimeByPiece;
    std::unordered_map<ObjectGuid, std::unordered_map<ObjectGuid, time_t>> chessFailedMoveTriggerUntilByPiece;
    std::unordered_map<ObjectGuid, time_t> chessLastMoveCommandByPiece;
    std::unordered_map<ObjectGuid, time_t> chessLastAbilityCommandByPiece;
    std::unordered_map<ObjectGuid, uint32> chessLastAnyCommandMsByPiece;
    std::unordered_map<ObjectGuid, ObjectGuid> chessOffensiveTargetByPiece;
    std::unordered_map<ObjectGuid, ObjectGuid> chessSupportTargetByPiece;
    std::unordered_map<ObjectGuid, ChessTargetFailureState> chessTargetFailureByPiece;
    std::unordered_map<ObjectGuid, ChessDiagnosticState> chessDiagnosticByPiece;
    std::unordered_map<ObjectGuid, ChessSquare> chessLastEnemyKingSquareByBot;
    std::unordered_map<uint32, time_t> chessEventStartByInstance;
    std::unordered_map<ObjectGuid, ChessSquare> chessLastSquareByPiece;
    std::unordered_map<ObjectGuid, ChessSquare> chessLastMoveFromSquareByPiece;
    std::unordered_map<ObjectGuid, ChessSquare> chessLastMoveToSquareByPiece;
    std::unordered_map<ObjectGuid, ChessPendingMove> chessPendingMoveByPiece;
    std::unordered_map<ObjectGuid, ChessPendingClaim> chessPendingClaimByBot;
    std::unordered_map<ObjectGuid, std::unordered_map<uint32, ChessAbilityNoOpBackoff>> chessAbilityNoOpBackoffByPiece;
    std::unordered_map<ObjectGuid, time_t> chessMovementCooldownUntilByPiece;
    std::unordered_map<ObjectGuid, time_t> chessOpeningMoveRetryUntilByPiece;
    std::unordered_map<ObjectGuid, time_t> chessReclaimSuppressedUntilByPiece;
    std::unordered_map<uint32, std::set<ObjectGuid>> chessOpenedLanePawnsByInstance;
    std::unordered_set<uint32> chessOpeningProgressConfirmedByInstance;
    struct ChessBoardState
    {
        std::map<std::pair<int, int>, Creature*> squareToTrigger;
        std::unordered_map<ObjectGuid, ChessSquare> pieceSquare;
        std::set<std::pair<int, int>> occupied;
        float originX = -11108.099609f;
        float originY = -1872.910034f;
        float rowStepX = 4.4f;
        float rowStepY = 3.45f;
        float colStepX = 3.49f;
        float colStepY = -4.4f;
        int minRow = 0;
        int maxRow = 7;
        int minCol = 0;
        int maxCol = 7;
        uint32 cacheStampMs = 0;
    };

    std::unordered_map<ObjectGuid, time_t> chessPoisonCloudLastAppliedByTarget;
    std::unordered_map<ObjectGuid, ChessBoardState> chessBoardCacheByBot;
    constexpr uint32 ChessBoardCacheTtlMs = 250;
    constexpr uint32 SpellChangeFacing = 30284;
    constexpr float ChessAutomaticMeleeRange = 10.0f;
    constexpr float ChessConeAngle = static_cast<float>(M_PI) / 3.0f;

    enum class ChessPhase : uint8
    {
        OPENING = 0,
        CLAIM_HIGH_VALUE = 1,
        COMBAT = 2
    };

    static bool HasPawnMovedDuringOpening(uint32 instanceId, ObjectGuid const& pieceGuid)
    {
        if (!instanceId)
            return false;

        auto it = chessOpenedLanePawnsByInstance.find(instanceId);
        return it != chessOpenedLanePawnsByInstance.end() && it->second.find(pieceGuid) != it->second.end();
    }

    static bool HasConfirmedOpeningProgress(uint32 instanceId)
    {
        return instanceId != 0 && chessOpeningProgressConfirmedByInstance.find(instanceId) != chessOpeningProgressConfirmedByInstance.end();
    }

    static bool IsOpeningChessPawnFile(ChessSquare const& square)
    {
        // Open both rooks and bishops. Queens, kings, and knights can then use the adjacent gaps
        // allowed by Karazhan's box-shaped movement rules.
        return square.col == 0 || square.col == 2 || square.col == 5 || square.col == 7;
    }

    static bool IsPawnEntry(uint32 e);
    static bool IsKingEntry(uint32 e);
    static bool IsSummonedDaemonChessPiece(uint32 entry);
    static bool IsOrcWarlockChessPiece(uint32 entry);
    static void PurgeChessPieceCacheForGuid(Player* bot, ObjectGuid const& pieceGuid, std::string const& source);

    static bool IsInsideBoard(ChessBoardState const& b, int row, int col);
    static bool WorldToChessSquare(ChessBoardState const& s, float x, float y, int& row, int& col);
    static Position ChessSquareToWorld(ChessBoardState const& s, int row, int col, float z);

    static bool IsChessDiagnosticsEnabled()
    {
        return sConfigMgr->GetOption<bool>("AiPlayerbot.KarazhanChess.Diagnostics", false);
    }

    static void LogChessDecision(Player* bot, Creature* piece, std::string const& decision,
        std::string const& details, bool force = false)
    {
        if (!IsChessDiagnosticsEnabled() || !bot || !piece)
            return;

        uint32 const nowMs = getMSTime();
        std::string const fingerprint = decision + ":" + details;
        ChessDiagnosticState& state = chessDiagnosticByPiece[piece->GetGUID()];
        if (!force && state.fingerprint == fingerprint && (nowMs - state.lastLoggedMs) < 10000)
            return;

        state.fingerprint = fingerprint;
        state.lastLoggedMs = nowMs;
        LOG_INFO("playerbots.chess", "bot={} piece={} guid={} decision={} {}", bot->GetName(), piece->GetName(),
            piece->GetGUID().ToString().c_str(), decision, details);
    }

    static void MarkOpeningProgressConfirmed(uint32 instanceId, Player* bot, Creature* piece, ChessSquare const& from, ChessSquare const& to, std::string const& source)
    {
        if (!instanceId || !piece)
            return;

        if (!chessOpeningProgressConfirmedByInstance.insert(instanceId).second)
            return;

    }

    static ChessPendingClaim* GetPendingChessClaim(Player* bot)
    {
        if (!bot)
            return nullptr;

        auto it = chessPendingClaimByBot.find(bot->GetGUID());
        return it != chessPendingClaimByBot.end() ? &it->second : nullptr;
    }

    static void ClearPendingChessClaim(Player* bot, std::string const& reason)
    {
        if (!bot)
            return;

        auto it = chessPendingClaimByBot.find(bot->GetGUID());
        if (it == chessPendingClaimByBot.end())
            return;

        Creature* target = nullptr;
        if (Unit* unit = ObjectAccessor::GetUnit(*bot, it->second.pieceGuid))
            target = unit->ToCreature();

        chessPendingClaimByBot.erase(it);
    }

    static std::string GetChessSpellName(uint32 spellId)
    {
        if (!spellId)
            return "none";

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId))
        {
            if (spellInfo->SpellName[0])
                return spellInfo->SpellName[0];
        }

        return "unknown";
    }

    static std::string ToLowerCopy(std::string s)
    {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return s;
    }

    static bool ContainsChessSpellNeedle(std::string const& haystack, char const* needle)
    {
        if (!needle || !*needle)
            return false;

        return haystack.find(needle) != std::string::npos;
    }

    static bool IsExplicitOffensiveChessSpell(uint32 spellId)
    {
        switch (spellId)
        {
            case 37406: // Heroic Blow
            case 37413: // Vicious Strike
            case 37427: // Geyser
            case 37428: // Hellfire
            case 37453: // Smash
            case 37454: // Bite
            case 37459: // Holy Lance
            case 37461: // Shadow Spear
            case 37462: // Elemental Blast
            case 37463: // Fireball
            case 37465: // Rain of Fire
            case 37469: // Poison Cloud
            case 37474: // Sweep
            case 37476: // Cleave
            case 37498: // Stomp
            case 37502: // Howl
                return true;
            default:
                return false;
        }
    }

    static bool IsCasterCenteredOffensiveChessSpell(uint32 spellId)
    {
        switch (spellId)
        {
            case 37427: // Geyser
            case 37428: // Hellfire
            case 37474: // Sweep
            case 37476: // Cleave
                return true;
            default:
                return false;
        }
    }

    static bool IsConeOffensiveChessSpell(uint32 spellId)
    {
        switch (spellId)
        {
            case 37406: // Heroic Blow
            case 37413: // Vicious Strike
            case 37453: // Smash
            case 37454: // Bite
            case 37459: // Holy Lance
            case 37461: // Shadow Spear
            case 37498: // Stomp
            case 37502: // Howl
                return true;
            default:
                return false;
        }
    }

    static bool IsTargetAreaOffensiveChessSpell(uint32 spellId)
    {
        return spellId == 37465 || spellId == 37469; // Rain of Fire / Poison Cloud
    }

    static char const* GetChessSpellShapeLabel(uint32 spellId)
    {
        if (IsConeOffensiveChessSpell(spellId))
            return "cone";
        if (IsCasterCenteredOffensiveChessSpell(spellId))
            return "caster-area";
        if (IsTargetAreaOffensiveChessSpell(spellId))
            return "target-area";
        return "direct";
    }

    static bool IsChessSelfUtilitySpell(uint32 spellId)
    {
        switch (spellId)
        {
            case 37414: // Shield Block
            case 37416: // Weapon Deflection
            case 37432: // Water Shield
            case 37434: // Fire Shield
            case 37471: // Heroism
            case 37472: // Bloodlust
                return true;
            default:
                return false;
        }
    }

    static float GetChessSpellEffectRadius(Creature* piece, uint32 spellId)
    {
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo)
            return 0.0f;

        float radius = 0.0f;
        for (uint8 effect = 0; effect < MAX_SPELL_EFFECTS; ++effect)
            radius = std::max(radius, spellInfo->Effects[effect].CalcRadius(piece));
        return radius;
    }

    static float GetChessOffensiveSpellRange(Creature* piece, uint32 spellId)
    {
        if (!piece)
            return 0.0f;

        if (IsConeOffensiveChessSpell(spellId) || IsCasterCenteredOffensiveChessSpell(spellId))
            return GetChessSpellEffectRadius(piece, spellId);

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId))
            return spellInfo->GetMaxRange(false, piece);

        return 0.0f;
    }

    static float GetChessSpellMinRange(Creature* piece, uint32 spellId, bool positive = false)
    {
        if (!piece || IsConeOffensiveChessSpell(spellId) || IsCasterCenteredOffensiveChessSpell(spellId))
            return 0.0f;

        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId))
            return spellInfo->GetMinRange(positive);

        return 0.0f;
    }

    static bool CompleteChessSpellCast(Creature* piece, uint32 spellId, SpellCastResult result)
    {
        if (!piece || result != SPELL_CAST_OK)
            return false;

        // Server-side bot casts bypass the pet opcode path which normally records creature cooldowns.
        piece->AddSpellCooldown(spellId, 0, 0);
        return true;
    }

    static bool CastChessSpell(Creature* piece, Unit* target, uint32 spellId)
    {
        if (!piece || !target || !spellId || piece->HasSpellCooldown(spellId))
            return false;

        return CompleteChessSpellCast(piece, spellId, piece->CastSpell(target, spellId, true));
    }

    static bool CastOffensiveChessSpell(Creature* piece, Unit* target, uint32 spellId)
    {
        if (!piece || !spellId || piece->HasSpellCooldown(spellId))
            return false;

        if (IsCasterCenteredOffensiveChessSpell(spellId) || IsConeOffensiveChessSpell(spellId))
            return CastChessSpell(piece, piece, spellId);

        return target && CastChessSpell(piece, target, spellId);
    }

    static bool IsKingAttackOffensiveChessSpell(uint32 spellId, std::string& rejectReason)
    {
        rejectReason.clear();

        if (spellId == 37456) // Shadow Mend is support/heal only and must never be routed into enemy pressure.
        {
            rejectReason = "necrolyte_shadow_mend_support_only";
            return false;
        }

        std::string const lower = ToLowerCopy(GetChessSpellName(spellId));
        if (spellId == 0 || lower == "none" || lower == "unknown")
        {
            rejectReason = "non_offensive";
            return false;
        }

        if (IsExplicitOffensiveChessSpell(spellId))
            return true;

        if (ContainsChessSpellNeedle(lower, "move"))
        {
            rejectReason = "movement_spell";
            return false;
        }

        if (ContainsChessSpellNeedle(lower, "change facing"))
        {
            rejectReason = "facing_spell";
            return false;
        }

        if (ContainsChessSpellNeedle(lower, "shield") ||
            ContainsChessSpellNeedle(lower, "deflection") ||
            ContainsChessSpellNeedle(lower, "command") ||
            ContainsChessSpellNeedle(lower, "control") ||
            ContainsChessSpellNeedle(lower, "heal") ||
            ContainsChessSpellNeedle(lower, "buff") ||
            ContainsChessSpellNeedle(lower, "armor"))
        {
            rejectReason = "utility_spell";
            return false;
        }

        if (ContainsChessSpellNeedle(lower, "fireball") ||
            ContainsChessSpellNeedle(lower, "hellfire") ||
            ContainsChessSpellNeedle(lower, "poison") ||
            ContainsChessSpellNeedle(lower, "bite") ||
            ContainsChessSpellNeedle(lower, "vicious") ||
            ContainsChessSpellNeedle(lower, "strike") ||
            ContainsChessSpellNeedle(lower, "blast") ||
            ContainsChessSpellNeedle(lower, "bolt") ||
            ContainsChessSpellNeedle(lower, "smash") ||
            ContainsChessSpellNeedle(lower, "crush") ||
            ContainsChessSpellNeedle(lower, "cleave") ||
            ContainsChessSpellNeedle(lower, "shoot") ||
            ContainsChessSpellNeedle(lower, "arrow") ||
            ContainsChessSpellNeedle(lower, "spear") ||
            ContainsChessSpellNeedle(lower, "shock") ||
            ContainsChessSpellNeedle(lower, "curse"))
        {
            return true;
        }

        rejectReason = "non_offensive";
        return false;
    }

    static std::vector<uint32> GetLikelyOffensiveChessSpells(Creature* piece)
    {
        std::vector<uint32> spells;
        if (!piece)
            return spells;

        for (uint8 i = 0; i < MAX_CREATURE_SPELLS; ++i)
        {
            uint32 const spellId = piece->m_spells[i];
            if (!spellId)
                continue;

            std::string rejectReason;
            if (IsKingAttackOffensiveChessSpell(spellId, rejectReason))
                spells.push_back(spellId);
        }

        return spells;
    }

    static bool IsPoisonCloudChessSpell(uint32 spellId)
    {
        return spellId == 37469;
    }

    static bool IsChessHealSpellBlockedOnEnemy(uint32 spellId)
    {
        if (spellId == 37456)
            return true;

        std::string const lower = ToLowerCopy(GetChessSpellName(spellId));
        return ContainsChessSpellNeedle(lower, "heal") ||
               ContainsChessSpellNeedle(lower, "mend") ||
               ContainsChessSpellNeedle(lower, "restore") ||
               ContainsChessSpellNeedle(lower, "renew");
    }

    static char const* GetChessPieceRoleLabel(Creature* piece)
    {
        if (!piece)
            return "unknown";

        if (IsKingChessPieceEntry(piece->GetEntry()))
            return "king";
        if (IsHealerChessPieceEntry(piece->GetEntry()))
            return "healer";
        if (IsDamageChessPieceEntry(piece->GetEntry()))
            return "damage";
        if (IsPawnEntry(piece->GetEntry()))
            return "pawn";
        if (IsSummonedDaemonChessPiece(piece->GetEntry()))
            return "daemon";
        if (IsOrcWarlockChessPiece(piece->GetEntry()))
            return "warlock";

        return "unknown";
    }

    static std::string ChessSquareToString(ChessSquare const& square)
    {
        return "(" + std::to_string(square.row) + "," + std::to_string(square.col) + ")";
    }

    static bool ShouldThrottlePoisonCloudCast(Player* bot, Creature* caster, Creature* target, uint32 spellId, time_t now, std::string& reason)
    {
        reason = "none";
        if (!caster || !target)
        {
            reason = "invalid_context";
            return true;
        }

        if (!IsPoisonCloudChessSpell(spellId))
            return false;

        bool const auraActive = target->HasAura(spellId);
        time_t const lastApplied = chessPoisonCloudLastAppliedByTarget.count(target->GetGUID()) ?
            chessPoisonCloudLastAppliedByTarget[target->GetGUID()] : 0;
        bool const recentlyApplied = lastApplied && (now - lastApplied) < 4;
        if (auraActive || recentlyApplied)
        {
            reason = auraActive ? "target_aura_active" : "recently_applied";
            return true;
        }

        return false;
    }

    static uint32 CountNearbyEnemyChessPieces(PlayerbotAI* botAI, Player* bot, Creature* piece, float radius)
    {
        if (!botAI || !bot || !piece)
            return 0;

        uint32 count = 0;
        for (Creature* candidate : GetNearbyChessPieces(botAI, bot, false))
        {
            if (!candidate || !candidate->IsAlive())
                continue;

            if (!IsEnemyChessPieceForBot(bot, candidate))
                continue;

            if (IsKingChessPieceEntry(candidate->GetEntry()))
                continue;

            if (piece->GetExactDist2d(candidate) <= radius)
                ++count;
        }

        return count;
    }

    static bool IsChessSpellNoOpBackoffActive(ObjectGuid const& pieceGuid, uint32 spellId, time_t now, time_t& remainingOut)
    {
        remainingOut = 0;
        auto pieceIt = chessAbilityNoOpBackoffByPiece.find(pieceGuid);
        if (pieceIt == chessAbilityNoOpBackoffByPiece.end())
            return false;

        auto spellIt = pieceIt->second.find(spellId);
        if (spellIt == pieceIt->second.end())
            return false;

        if (now >= spellIt->second.until)
            return false;

        remainingOut = spellIt->second.until - now;
        return true;
    }

    static void StampChessSpellNoOpBackoff(ObjectGuid const& pieceGuid, uint32 spellId, time_t now, time_t seconds)
    {
        if (!pieceGuid || !spellId)
            return;

        chessAbilityNoOpBackoffByPiece[pieceGuid][spellId].until = now + seconds;
    }

    static void ClearChessSpellNoOpBackoff(ObjectGuid const& pieceGuid, uint32 spellId)
    {
        auto pieceIt = chessAbilityNoOpBackoffByPiece.find(pieceGuid);
        if (pieceIt == chessAbilityNoOpBackoffByPiece.end())
            return;

        pieceIt->second.erase(spellId);
        if (pieceIt->second.empty())
            chessAbilityNoOpBackoffByPiece.erase(pieceIt);
    }

    struct ChessOffensiveTargetSelection
    {
        Creature* target = nullptr;
        std::string category = "none";
        std::string rejectReason = "no_candidate";
        float distance = std::numeric_limits<float>::max();
    };

    struct ChessSupportTargetSelection
    {
        Creature* target = nullptr;
        std::string rejectReason = "no_candidate";
        float healthPct = 100.0f;
        float distance = std::numeric_limits<float>::max();
    };

    static bool IsActiveBoardEnemyChessPiece(Player* bot, ChessBoardState const& board, Creature* target, bool allowKing,
        ChessSquare& targetSquare, bool& targetActiveBoardPiece, std::string& rejectReason)
    {
        targetSquare = ChessSquare{ -1, -1 };
        targetActiveBoardPiece = false;

        if (!target)
        {
            rejectReason = "no-target";
            return false;
        }

        if (!target->IsInWorld())
        {
            rejectReason = "not-in-world";
            return false;
        }

        if (!target->IsAlive())
        {
            rejectReason = "dead";
            return false;
        }

        if (!IsChessPieceEntry(target->GetEntry()))
        {
            rejectReason = "not-chess-piece";
            return false;
        }

        if (!IsEnemyChessPieceForBot(bot, target))
        {
            rejectReason = "wrong-side";
            return false;
        }

        if (IsKingChessPieceEntry(target->GetEntry()) && !allowKing)
        {
            rejectReason = "king-deferred";
            return false;
        }

        auto boardIt = board.pieceSquare.find(target->GetGUID());
        if (boardIt == board.pieceSquare.end())
        {
            rejectReason = "target_missing_from_board";
            return false;
        }

        int row = -1;
        int col = -1;
        if (!WorldToChessSquare(board, target->GetPositionX(), target->GetPositionY(), row, col) ||
            !IsInsideBoard(board, row, col))
        {
            rejectReason = "target_sideline_detected";
            return false;
        }

        if (boardIt->second.row != row || boardIt->second.col != col)
        {
            rejectReason = "target_missing_from_board";
            return false;
        }

        targetSquare = boardIt->second;
        targetActiveBoardPiece = board.occupied.find({ row, col }) != board.occupied.end();
        if (!targetActiveBoardPiece)
        {
            rejectReason = "target_rejected_not_active_board_piece";
            return false;
        }

        rejectReason = "none";
        return true;
    }

    static bool IsActiveBoardFriendlyControlledChessPiece(
        Player* bot, ChessBoardState const& board, Creature* piece, ChessSquare& pieceSquare, std::string& rejectReason)
    {
        pieceSquare = ChessSquare{ -1, -1 };
        if (!piece)
        {
            rejectReason = "no-piece";
            return false;
        }

        if (!piece->IsInWorld())
        {
            rejectReason = "not-in-world";
            return false;
        }

        if (!piece->IsAlive())
        {
            rejectReason = "dead";
            return false;
        }

        if (!IsChessPieceEntry(piece->GetEntry()))
        {
            rejectReason = "not-chess-piece";
            return false;
        }

        if (!IsFriendlyChessPieceForBot(bot, piece))
        {
            rejectReason = "wrong-side";
            return false;
        }

        auto pieceIt = board.pieceSquare.find(piece->GetGUID());
        if (pieceIt == board.pieceSquare.end())
        {
            rejectReason = "piece_missing_from_board";
            return false;
        }

        int row = -1;
        int col = -1;
        if (!WorldToChessSquare(board, piece->GetPositionX(), piece->GetPositionY(), row, col) ||
            !IsInsideBoard(board, row, col))
        {
            rejectReason = "piece_sideline_detected";
            return false;
        }

        if (pieceIt->second.row != row || pieceIt->second.col != col)
        {
            rejectReason = "piece_square_mismatch";
            return false;
        }

        if (board.occupied.find({ row, col }) == board.occupied.end())
        {
            rejectReason = "piece_not_active_board_occupied";
            return false;
        }

        pieceSquare = pieceIt->second;
        rejectReason = "none";
        return true;
    }

    static bool HandleInvalidControlledChessPiece(
        PlayerbotAI* botAI, Player* bot, Creature* piece, ChessBoardState const& board, std::string const& source, std::string const& reason)
    {
        if (!bot || !piece)
            return false;

        ChessSquare boardSq{ -1, -1 };
        auto const sqIt = board.pieceSquare.find(piece->GetGUID());
        if (sqIt != board.pieceSquare.end())
            boardSq = sqIt->second;

        PurgeChessPieceCacheForGuid(bot, piece->GetGUID(), source + "-invalid-controlled-piece");
        chessSelfAbilityThrottleByPiece.erase(piece->GetGUID());
        chessLastEnemyKingSquareByBot.erase(bot->GetGUID());
        ClearPendingChessClaim(bot, source + "-invalid-controlled-piece");
        ClearAssignedChessPiece(bot);

        if (bot->GetCharm() && bot->GetCharm()->GetGUID() == piece->GetGUID())
            piece->RemoveCharmedBy(bot);

        return true;
    }

    static uint32 CountNearbyActiveBoardEnemyChessPieces(Player* bot, ChessBoardState const& board, Creature* piece, float radius)
    {
        if (!bot || !piece)
            return 0;

        uint32 count = 0;
        for (auto const& boardEntry : board.pieceSquare)
        {
            Creature* candidate = ObjectAccessor::GetCreature(*piece, boardEntry.first);
            if (!candidate || candidate == piece)
                continue;

            ChessSquare targetSquare;
            bool targetActiveBoardPiece = false;
            std::string rejectReason;
            if (!IsActiveBoardEnemyChessPiece(bot, board, candidate, false, targetSquare, targetActiveBoardPiece, rejectReason))
                continue;

            if (piece->GetExactDist2d(candidate) <= radius)
                ++count;
        }

        return count;
    }

    static void CountActiveBoardEnemyChessPieces(Player* bot, ChessBoardState const& board, uint32& supportAlive, uint32& damageAlive, uint32& pawnAlive)
    {
        supportAlive = 0;
        damageAlive = 0;
        pawnAlive = 0;

        if (!bot)
            return;

        uint32 activeNonKing = 0;
        for (auto const& boardEntry : board.pieceSquare)
        {
            Creature* candidate = ObjectAccessor::GetCreature(*bot, boardEntry.first);
            if (!candidate || IsKingChessPieceEntry(candidate->GetEntry()))
                continue;

            ChessSquare targetSquare;
            bool targetActiveBoardPiece = false;
            std::string rejectReason;
            if (!IsActiveBoardEnemyChessPiece(bot, board, candidate, false, targetSquare, targetActiveBoardPiece, rejectReason))
            {
                if (candidate->IsInWorld() && candidate->IsAlive() && IsChessPieceEntry(candidate->GetEntry()) && IsEnemyChessPieceForBot(bot, candidate))
                {
                }
                continue;
            }

            ++activeNonKing;
            if (IsHealerChessPieceEntry(candidate->GetEntry()))
                ++supportAlive;
            else if (IsDamageChessPieceEntry(candidate->GetEntry()))
                ++damageAlive;
            else if (IsPawnEntry(candidate->GetEntry()))
                ++pawnAlive;
            else
                ++damageAlive;
        }

    }

    static bool IsKarazhanChessKingFocusAllowedActiveBoard(Player* bot, ChessBoardState const& board, Creature* enemyKing,
        uint32& supportAlive, uint32& damageAlive, uint32& pawnAlive, std::string& gateReason)
    {
        CountActiveBoardEnemyChessPieces(bot, board, supportAlive, damageAlive, pawnAlive);

        bool const kingCriticallyLow = enemyKing && enemyKing->GetHealthPct() < 30.0f;
        bool const boardMostlyCleared = supportAlive == 0 && damageAlive == 0 && pawnAlive <= 3;
        gateReason = kingCriticallyLow ? "king_critically_low" : (boardMostlyCleared ? "board_mostly_cleared" : "board_not_cleared");
        return kingCriticallyLow || boardMostlyCleared;
    }

    static bool IsNoActionableNonKingRejectReason(std::string const& reason)
    {
        return reason.rfind("no_active_board_target", 0) == 0 ||
               reason == "out_of_range" ||
               reason == "target_temporarily_unreachable" ||
               reason == "no_reachable_attack_square" ||
               reason == "no_enemy_candidate" ||
               reason == "no_valid_candidate";
    }

    static bool IsChessSupportSpell(uint32 spellId, std::string& rejectReason)
    {
        if (IsExplicitOffensiveChessSpell(spellId))
        {
            rejectReason = "offensive_spell";
            return false;
        }

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        std::string lower = spellInfo ? spellInfo->SpellName[0] : std::string();
        std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

        if (spellId == SPELL_MOVE_COOLDOWN || ContainsChessSpellNeedle(lower, "move") ||
            ContainsChessSpellNeedle(lower, "facing") || ContainsChessSpellNeedle(lower, "control") ||
            ContainsChessSpellNeedle(lower, "command"))
        {
            rejectReason = "movement_or_control_spell";
            return false;
        }

        if (ContainsChessSpellNeedle(lower, "heal") || ContainsChessSpellNeedle(lower, "mend") ||
            ContainsChessSpellNeedle(lower, "restore") || ContainsChessSpellNeedle(lower, "renew") ||
            ContainsChessSpellNeedle(lower, "holy"))
        {
            rejectReason = "none";
            return true;
        }

        rejectReason = "not_support_spell";
        return false;
    }

    static ChessSupportTargetSelection SelectDamagedFriendlyActiveBoardTarget(
        Player* bot, ChessBoardState const& board, Creature* piece)
    {
        ChessSupportTargetSelection selection;
        if (!bot || !piece)
        {
            selection.rejectReason = "invalid_context";
            return selection;
        }

        bool sawFriendly = false;
        bool sawDamaged = false;
        float bestScore = -std::numeric_limits<float>::max();
        for (auto const& boardEntry : board.pieceSquare)
        {
            Creature* candidate = ObjectAccessor::GetCreature(*piece, boardEntry.first);
            if (!candidate || !candidate->IsInWorld() || !candidate->IsAlive() || !IsChessPieceEntry(candidate->GetEntry()) ||
                !IsFriendlyChessPieceForBot(bot, candidate))
                continue;

            auto occupiedIt = board.occupied.find({ boardEntry.second.row, boardEntry.second.col });
            if (occupiedIt == board.occupied.end())
                continue;

            sawFriendly = true;
            uint32 const missingHealth = candidate->GetMaxHealth() - candidate->GetHealth();
            float const hpPct = candidate->GetHealthPct();
            float const distance = piece->GetExactDist2d(candidate);
            if (missingHealth < 3000 && hpPct > 90.0f)
                continue;

            sawDamaged = true;
            auto failureIt = chessTargetFailureByPiece.find(piece->GetGUID());
            if (failureIt != chessTargetFailureByPiece.end() &&
                failureIt->second.targetGuid == candidate->GetGUID() &&
                std::time(nullptr) < failureIt->second.blockedUntil)
                continue;

            float score = static_cast<float>(missingHealth) + (100.0f - hpPct) * 500.0f - distance * 20.0f;
            if (IsKingChessPieceEntry(candidate->GetEntry()))
                score += 15000.0f;

            auto persistent = chessSupportTargetByPiece.find(piece->GetGUID());
            if (persistent != chessSupportTargetByPiece.end() && persistent->second == candidate->GetGUID())
                score += 5000.0f;

            if (!selection.target || score > bestScore ||
                (score == bestScore && candidate->GetGUID() < selection.target->GetGUID()))
            {
                selection.target = candidate;
                selection.healthPct = hpPct;
                selection.distance = distance;
                selection.rejectReason = "none";
                bestScore = score;
            }
        }

        if (!selection.target)
        {
            chessSupportTargetByPiece.erase(piece->GetGUID());
            selection.rejectReason = sawDamaged ? "no_valid_support_target" :
                (sawFriendly ? "no_damaged_friendly" : "no_friendly_active_board_piece");
        }
        else
            chessSupportTargetByPiece[piece->GetGUID()] = selection.target->GetGUID();

        return selection;
    }

    static int GetChessPieceMoveDelta(uint32 entry)
    {
        if (entry == NPC_QUEEN_A || entry == NPC_QUEEN_H)
            return 3;
        if (entry == NPC_KNIGHT_A || entry == NPC_KNIGHT_H)
            return 2;
        return 1;
    }

    static bool HasReachableChessAttackSquare(Creature* piece, Creature* target, ChessBoardState const& board)
    {
        if (!piece || !target)
            return false;

        auto startIt = board.pieceSquare.find(piece->GetGUID());
        if (startIt == board.pieceSquare.end())
            return false;

        std::vector<ChessSquare> pending{ startIt->second };
        std::set<std::pair<int, int>> visited{ { startIt->second.row, startIt->second.col } };
        int const maxDelta = GetChessPieceMoveDelta(piece->GetEntry());
        std::vector<uint32> const offensiveSpells = GetLikelyOffensiveChessSpells(piece);
        for (size_t index = 0; index < pending.size(); ++index)
        {
            ChessSquare const square = pending[index];
            Position const position = ChessSquareToWorld(board, square.row, square.col, piece->GetPositionZ());
            float const dx = position.GetPositionX() - target->GetPositionX();
            float const dy = position.GetPositionY() - target->GetPositionY();
            float const distance = std::sqrt(dx * dx + dy * dy);
            if (distance <= ChessAutomaticMeleeRange)
                return true;

            for (uint32 spellId : offensiveSpells)
            {
                float const maxRange = GetChessOffensiveSpellRange(piece, spellId);
                float const minRange = GetChessSpellMinRange(piece, spellId);
                if (distance >= minRange && (maxRange <= 0.0f || distance <= maxRange))
                    return true;
            }

            for (int rowDelta = -maxDelta; rowDelta <= maxDelta; ++rowDelta)
            {
                for (int colDelta = -maxDelta; colDelta <= maxDelta; ++colDelta)
                {
                    if (!rowDelta && !colDelta)
                        continue;

                    int const row = square.row + rowDelta;
                    int const col = square.col + colDelta;
                    std::pair<int, int> const destination{ row, col };
                    if (!IsInsideBoard(board, row, col) || visited.find(destination) != visited.end())
                        continue;
                    if (board.occupied.find(destination) != board.occupied.end())
                        continue;

                    visited.insert(destination);
                    pending.push_back(ChessSquare{ row, col });
                }
            }
        }

        return false;
    }

    static ChessOffensiveTargetSelection SelectNonKingChessTarget(PlayerbotAI* botAI, Player* bot, Creature* piece, ChessBoardState const& board, char const* source)
    {
        ChessOffensiveTargetSelection selection;
        if (!botAI || !bot || !piece)
        {
            selection.rejectReason = "invalid_context";
            return selection;
        }

        (void)source;
        auto pieceSquareIt = board.pieceSquare.find(piece->GetGUID());
        if (pieceSquareIt == board.pieceSquare.end())
        {
            selection.rejectReason = "piece_missing_from_board";
            return selection;
        }

        float bestScore = -std::numeric_limits<float>::max();
        bool sawEnemy = false;
        bool sawInactive = false;
        bool sawTemporarilyBlocked = false;
        bool sawUnreachable = false;
        std::string lastInactiveReason = "none";

        for (Creature* candidate : GetNearbyChessPieces(botAI, bot, false))
        {
            if (!candidate)
                continue;

            if (IsKingChessPieceEntry(candidate->GetEntry()))
                continue;

            ChessSquare targetSquare;
            bool targetActiveBoardPiece = false;
            std::string activeRejectReason;
            if (!IsActiveBoardEnemyChessPiece(bot, board, candidate, false, targetSquare, targetActiveBoardPiece, activeRejectReason))
            {
                if (candidate && candidate->IsInWorld() && candidate->IsAlive() && IsChessPieceEntry(candidate->GetEntry()) && IsEnemyChessPieceForBot(bot, candidate))
                {
                    sawEnemy = true;
                    sawInactive = true;
                    lastInactiveReason = activeRejectReason;
                }
                continue;
            }

            sawEnemy = true;
            float const distance = piece->GetExactDist2d(candidate);
            auto failureIt = chessTargetFailureByPiece.find(piece->GetGUID());
            if (failureIt != chessTargetFailureByPiece.end() &&
                failureIt->second.targetGuid == candidate->GetGUID() &&
                std::time(nullptr) < failureIt->second.blockedUntil)
            {
                sawTemporarilyBlocked = true;
                continue;
            }

            if (!HasReachableChessAttackSquare(piece, candidate, board))
            {
                sawUnreachable = true;
                continue;
            }

            int const gridDistance = std::max(
                std::abs(targetSquare.row - pieceSquareIt->second.row),
                std::abs(targetSquare.col - pieceSquareIt->second.col));
            float score = IsHealerChessPieceEntry(candidate->GetEntry()) ? 30000.0f :
                (IsPawnEntry(candidate->GetEntry()) ? 10000.0f : 20000.0f);
            score += (100.0f - candidate->GetHealthPct()) * 30.0f;
            score -= static_cast<float>(gridDistance) * 250.0f;
            score -= distance * 5.0f;

            if (!selection.target || score > bestScore ||
                (score == bestScore && candidate->GetGUID() < selection.target->GetGUID()))
            {
                bestScore = score;
                selection.target = candidate;
                selection.category = IsHealerChessPieceEntry(candidate->GetEntry()) ? "support" :
                    (IsPawnEntry(candidate->GetEntry()) ? "pawn" : "damage");
                selection.distance = distance;
                selection.rejectReason = "none";
            }
        }

        if (selection.target)
            return selection;

        if (!sawEnemy)
            selection.rejectReason = "no_enemy_candidate";
        else if (sawTemporarilyBlocked)
            selection.rejectReason = "target_temporarily_unreachable";
        else if (sawUnreachable)
            selection.rejectReason = "no_reachable_attack_square";
        else if (sawInactive)
            selection.rejectReason = "no_active_board_target:" + lastInactiveReason;
        else
            selection.rejectReason = "no_valid_candidate";

        if (!selection.target && sawInactive)
        {
        }

        return selection;
    }

    static bool IsSummonedDaemonChessPiece(uint32 entry)
    {
        return entry == NPC_ROOK_H;
    }

    static bool IsOrcWarlockChessPiece(uint32 entry)
    {
        return entry == NPC_QUEEN_H;
    }

    static bool IsChessSpellTargetInRange(Creature* piece, Unit* target, uint32 spellId)
    {
        if (!piece || !target)
            return false;

        bool const positive = spellId == 37455 || spellId == 37456;
        float maxRange = 0.0f;
        float minRange = 0.0f;
        if (IsExplicitOffensiveChessSpell(spellId))
        {
            maxRange = GetChessOffensiveSpellRange(piece, spellId);
            minRange = GetChessSpellMinRange(piece, spellId, false);
        }
        else if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId))
        {
            maxRange = spellInfo->GetMaxRange(positive, piece);
            minRange = spellInfo->GetMinRange(positive);
        }

        float const distance = piece->GetExactDist2d(target);
        if (distance < minRange || (maxRange > 0.0f && distance > maxRange))
            return false;

        return !IsConeOffensiveChessSpell(spellId) || piece->HasInArc(ChessConeAngle, target);
    }

    static ChessOffensiveTargetSelection SelectPersistentChessTarget(
        PlayerbotAI* botAI, Player* bot, Creature* piece, ChessBoardState const& board, char const* source)
    {
        ChessOffensiveTargetSelection selection;
        if (!botAI || !bot || !piece)
        {
            selection.rejectReason = "invalid_context";
            return selection;
        }

        auto targetIt = chessOffensiveTargetByPiece.find(piece->GetGUID());
        if (targetIt != chessOffensiveTargetByPiece.end())
        {
            Creature* target = ObjectAccessor::GetCreature(*piece, targetIt->second);
            ChessSquare targetSquare;
            bool targetActiveBoardPiece = false;
            std::string rejectReason;
            if (IsActiveBoardEnemyChessPiece(
                    bot, board, target, false, targetSquare, targetActiveBoardPiece, rejectReason) &&
                HasReachableChessAttackSquare(piece, target, board))
            {
                selection.target = target;
                selection.category = IsHealerChessPieceEntry(target->GetEntry()) ? "support" :
                    (IsPawnEntry(target->GetEntry()) ? "pawn" : "damage");
                selection.distance = piece->GetExactDist2d(target);
                selection.rejectReason = "none";
                return selection;
            }

            if (rejectReason == "none")
                rejectReason = "no_reachable_attack_square";

            LogChessDecision(bot, piece, "target-reacquire",
                "source=" + std::string(source) + " old_target=" + targetIt->second.ToString() +
                " reason=" + rejectReason, true);
            chessOffensiveTargetByPiece.erase(targetIt);
        }

        selection = SelectNonKingChessTarget(botAI, bot, piece, board, source);
        if (selection.target)
        {
            chessOffensiveTargetByPiece[piece->GetGUID()] = selection.target->GetGUID();
            auto pieceSquare = board.pieceSquare.find(piece->GetGUID());
            auto targetSquare = board.pieceSquare.find(selection.target->GetGUID());
            LogChessDecision(bot, piece, "target-acquired",
                "source=" + std::string(source) + " target=" + selection.target->GetName() + " target_guid=" +
                selection.target->GetGUID().ToString() + " category=" + selection.category + " piece_square=" +
                (pieceSquare != board.pieceSquare.end() ? ChessSquareToString(pieceSquare->second) : "unknown") +
                " target_square=" + (targetSquare != board.pieceSquare.end() ?
                ChessSquareToString(targetSquare->second) : "unknown"), true);
        }

        return selection;
    }

    static void PurgeChessPieceCacheForGuid(Player* bot, ObjectGuid const& pieceGuid, std::string const& source)
    {
        chessLastSquareByPiece.erase(pieceGuid);
        chessLastMoveFromSquareByPiece.erase(pieceGuid);
        chessLastMoveToSquareByPiece.erase(pieceGuid);
        chessLastFailedMoveTriggerByPiece.erase(pieceGuid);
        chessLastFailedMoveTimeByPiece.erase(pieceGuid);
        chessFailedMoveTriggerUntilByPiece.erase(pieceGuid);
        chessLastMoveCommandByPiece.erase(pieceGuid);
        chessLastAbilityCommandByPiece.erase(pieceGuid);
        chessLastAnyCommandMsByPiece.erase(pieceGuid);
        chessOffensiveTargetByPiece.erase(pieceGuid);
        chessSupportTargetByPiece.erase(pieceGuid);
        chessTargetFailureByPiece.erase(pieceGuid);
        chessDiagnosticByPiece.erase(pieceGuid);
        chessMovementCooldownUntilByPiece.erase(pieceGuid);
        chessOpeningMoveRetryUntilByPiece.erase(pieceGuid);
        chessReclaimSuppressedUntilByPiece.erase(pieceGuid);
        chessAbilityNoOpBackoffByPiece.erase(pieceGuid);

        if (chessPendingMoveByPiece.erase(pieceGuid))
        {
        }

        for (auto it = chessOpenedLanePawnsByInstance.begin(); it != chessOpenedLanePawnsByInstance.end(); )
        {
            it->second.erase(pieceGuid);
            if (it->second.empty())
                it = chessOpenedLanePawnsByInstance.erase(it);
            else
                ++it;
        }
    }

    static void CleanupStaleChessState(PlayerbotAI* botAI, Player* bot, std::string const& source)
    {
        if (!botAI || !bot)
            return;

        std::unordered_set<ObjectGuid> pieceGuids;
        auto collectPieceGuid = [&pieceGuids](auto const& map)
        {
            for (auto const& entry : map)
                pieceGuids.insert(entry.first);
        };

        collectPieceGuid(chessLastSquareByPiece);
        collectPieceGuid(chessLastMoveFromSquareByPiece);
        collectPieceGuid(chessLastMoveToSquareByPiece);
        collectPieceGuid(chessPendingMoveByPiece);
        collectPieceGuid(chessLastFailedMoveTriggerByPiece);
        collectPieceGuid(chessLastFailedMoveTimeByPiece);
        collectPieceGuid(chessFailedMoveTriggerUntilByPiece);
        collectPieceGuid(chessLastMoveCommandByPiece);
        collectPieceGuid(chessLastAbilityCommandByPiece);
        collectPieceGuid(chessLastAnyCommandMsByPiece);
        collectPieceGuid(chessOffensiveTargetByPiece);
        collectPieceGuid(chessSupportTargetByPiece);
        collectPieceGuid(chessTargetFailureByPiece);
        collectPieceGuid(chessDiagnosticByPiece);
        collectPieceGuid(chessMovementCooldownUntilByPiece);
        collectPieceGuid(chessOpeningMoveRetryUntilByPiece);
        collectPieceGuid(chessReclaimSuppressedUntilByPiece);
        collectPieceGuid(chessAbilityNoOpBackoffByPiece);

        for (auto const& pending : chessPendingClaimByBot)
            pieceGuids.insert(pending.second.pieceGuid);
        for (auto const& assigned : chessAssignedBotByPiece)
            pieceGuids.insert(assigned.first);
        for (auto const& instanceEntry : chessOpenedLanePawnsByInstance)
            for (ObjectGuid const& guid : instanceEntry.second)
                pieceGuids.insert(guid);

        size_t staleGuidCount = 0;
        for (ObjectGuid const& guid : pieceGuids)
        {
            Creature* piece = botAI->GetCreature(guid);
            bool const liveValid = piece && piece->IsInWorld() && piece->IsAlive() && IsChessPieceEntry(piece->GetEntry());
            if (liveValid)
                continue;

            ++staleGuidCount;
            PurgeChessPieceCacheForGuid(bot, guid, source);
        }

        size_t stalePendingClaims = 0;
        for (auto it = chessPendingClaimByBot.begin(); it != chessPendingClaimByBot.end(); )
        {
            Creature* pendingPiece = botAI->GetCreature(it->second.pieceGuid);
            bool const liveValid = pendingPiece && pendingPiece->IsInWorld() && pendingPiece->IsAlive() && IsChessPieceEntry(pendingPiece->GetEntry());
            if (!liveValid)
            {
                ++stalePendingClaims;
                it = chessPendingClaimByBot.erase(it);
                continue;
            }

            ++it;
        }

        size_t staleAssignments = 0;
        for (auto it = chessAssignedPieceByBot.begin(); it != chessAssignedPieceByBot.end(); )
        {
            Creature* assignedPiece = botAI->GetCreature(it->second);
            bool const liveValid = assignedPiece && assignedPiece->IsInWorld() && assignedPiece->IsAlive() && IsChessPieceEntry(assignedPiece->GetEntry());
            if (!liveValid)
            {
                ++staleAssignments;
                ObjectGuid const botGuid = it->first;
                ObjectGuid const pieceGuid = it->second;
                chessAssignedBotByPiece.erase(pieceGuid);
                chessAssignmentLockUntil.erase(botGuid);
                it = chessAssignedPieceByBot.erase(it);
                continue;
            }

            ++it;
        }

        for (auto it = chessAssignedBotByPiece.begin(); it != chessAssignedBotByPiece.end(); )
        {
            Creature* assignedPiece = botAI->GetCreature(it->first);
            bool const liveValid = assignedPiece && assignedPiece->IsInWorld() && assignedPiece->IsAlive() && IsChessPieceEntry(assignedPiece->GetEntry());
            if (!liveValid)
            {
                ++staleAssignments;
                it = chessAssignedBotByPiece.erase(it);
                continue;
            }

            ++it;
        }

        if (staleGuidCount || stalePendingClaims || staleAssignments)
        {
        }
    }

    static bool ValidateLiveChessTargetForCast(
        Player* bot, Creature* target, bool allowKing, std::string& rejectReason,
        bool& alive, bool& inWorld, bool& chessPiece, bool& enemySide, bool& isKing)
    {
        alive = target && target->IsAlive();
        inWorld = target && target->IsInWorld();
        chessPiece = target && IsChessPieceEntry(target->GetEntry());
        enemySide = target && IsEnemyChessPieceForBot(bot, target);
        isKing = target && IsKingChessPieceEntry(target->GetEntry());

        if (!target)
        {
            rejectReason = "no-target";
            return false;
        }

        if (!inWorld)
        {
            rejectReason = "not-in-world";
            return false;
        }

        if (!alive)
        {
            rejectReason = "dead";
            return false;
        }

        if (!chessPiece)
        {
            rejectReason = "not-chess-piece";
            return false;
        }

        if (!enemySide)
        {
            rejectReason = "wrong-side";
            return false;
        }

        if (isKing && !allowKing)
        {
            rejectReason = "king-deferred";
            return false;
        }

        rejectReason = "none";
        return true;
    }

    static bool ValidateLiveChessBoardTargetForCast(
        Player* bot, ChessBoardState const& board, Creature* target, bool allowKing,
        std::string& rejectReason, bool& alive, bool& inWorld, bool& chessPiece, bool& enemySide, bool& isKing,
        ChessSquare& targetSquare, bool& targetBoardSquareFound, bool& targetActiveBoardPiece)
    {
        targetSquare = ChessSquare{ -1, -1 };
        targetBoardSquareFound = false;
        targetActiveBoardPiece = false;

        if (!ValidateLiveChessTargetForCast(bot, target, allowKing, rejectReason, alive, inWorld, chessPiece, enemySide, isKing))
            return false;

        if (!target)
        {
            rejectReason = "no-target";
            return false;
        }

        if (!IsActiveBoardEnemyChessPiece(bot, board, target, allowKing, targetSquare, targetActiveBoardPiece, rejectReason))
            return false;

        targetBoardSquareFound = true;
        rejectReason = "none";
        return true;
    }

    static void SetPendingChessClaim(Player* bot, Creature* piece, uint32 instanceId, time_t now, time_t seconds = 30)
    {
        if (!bot || !piece)
            return;

        ChessPendingClaim pending;
        pending.pieceGuid = piece->GetGUID();
        pending.instanceId = instanceId;
        pending.createdAt = now;
        pending.expiresAt = now + seconds;
        pending.nextControlAttemptAt = now;
        pending.controlAttempts = 0;
        chessPendingClaimByBot[bot->GetGUID()] = pending;
    }

    static bool TryPendingChessClaimControl(Player* bot, Creature* piece, ChessPendingClaim& pending, time_t now)
    {
        if (!bot || !piece || pending.pieceGuid != piece->GetGUID() || now < pending.nextControlAttemptAt)
            return false;

        SpellCastResult const result = bot->CastSpell(piece, SPELL_CONTROL_PIECE, true);
        if (pending.controlAttempts < std::numeric_limits<uint8>::max())
            ++pending.controlAttempts;
        pending.nextControlAttemptAt = now + (result == SPELL_CAST_OK ? 3 : 2);
        LogChessDecision(bot, piece, "piece-control-attempt", "attempt=" +
            std::to_string(pending.controlAttempts) + " result=" +
            std::to_string(static_cast<uint32>(result)));
        return result == SPELL_CAST_OK;
    }

    static void SuppressReclaimForPiece(ObjectGuid const& pieceGuid, time_t now, time_t seconds = 20)
    {
        if (pieceGuid)
            chessReclaimSuppressedUntilByPiece[pieceGuid] = now + seconds;
    }

    static bool IsReclaimSuppressed(ObjectGuid const& pieceGuid, time_t now, std::string& reason)
    {
        auto it = chessReclaimSuppressedUntilByPiece.find(pieceGuid);
        if (it == chessReclaimSuppressedUntilByPiece.end())
            return false;

        if (now >= it->second)
        {
            chessReclaimSuppressedUntilByPiece.erase(it);
            return false;
        }

        reason = "reclaim-suppressed";
        return true;
    }

    static bool TryGetCachedChessSquare(ObjectGuid const& pieceGuid, ChessSquare& out, std::string& source)
    {
        auto itTo = chessLastMoveToSquareByPiece.find(pieceGuid);
        if (itTo != chessLastMoveToSquareByPiece.end())
        {
            out = itTo->second;
            source = "lastMoveTo";
            return true;
        }

        auto itLast = chessLastSquareByPiece.find(pieceGuid);
        if (itLast != chessLastSquareByPiece.end())
        {
            out = itLast->second;
            source = "lastSquare";
            return true;
        }

        auto itFrom = chessLastMoveFromSquareByPiece.find(pieceGuid);
        if (itFrom != chessLastMoveFromSquareByPiece.end())
        {
            out = itFrom->second;
            source = "lastMoveFrom";
            return true;
        }

        return false;
    }

    enum class ChessPendingMoveResult : uint8
    {
        NONE = 0,
        WAITING,
        CONFIRMED,
        EXPIRED
    };

    static void RegisterChessTargetFailure(Player* bot, Creature* piece, ObjectGuid const& targetGuid,
        time_t now, std::string const& reason)
    {
        if (!piece || !targetGuid)
            return;

        ChessTargetFailureState& failure = chessTargetFailureByPiece[piece->GetGUID()];
        if (failure.targetGuid != targetGuid)
        {
            failure.targetGuid = targetGuid;
            failure.failedMoves = 0;
        }
        ++failure.failedMoves;
        if (failure.failedMoves >= 2)
        {
            failure.blockedUntil = now + 15;
            chessOffensiveTargetByPiece.erase(piece->GetGUID());
            chessSupportTargetByPiece.erase(piece->GetGUID());
        }
        LogChessDecision(bot, piece, "target-progress-failed", "target_guid=" + targetGuid.ToString() +
            " reason=" + reason + " failures=" + std::to_string(failure.failedMoves) + " target_blocked=" +
            (failure.blockedUntil > now ? "1" : "0"), true);
    }

    static void RecordPendingChessMove(
        Player* bot, Creature* piece, uint32 instanceId, ChessSquare const& from, ChessSquare const& to,
        uint32 moveSpell, ObjectGuid const& triggerGuid, std::string const& source, time_t now, time_t expiresIn = 5,
        ObjectGuid const& tacticalTargetGuid = ObjectGuid::Empty)
    {
        if (!piece)
            return;

        ChessPendingMove pending;
        pending.fromSquare = from;
        pending.toSquare = to;
        pending.instanceId = instanceId;
        pending.createdAt = now;
        pending.expiresAt = now + expiresIn;
        pending.moveSpell = moveSpell;
        pending.triggerGuid = triggerGuid;
        pending.tacticalTargetGuid = tacticalTargetGuid;
        pending.source = source;
        chessPendingMoveByPiece[piece->GetGUID()] = pending;
        chessOpeningMoveRetryUntilByPiece[piece->GetGUID()] = pending.expiresAt;
        chessLastMoveCommandByPiece[piece->GetGUID()] = now;
        chessLastAnyCommandMsByPiece[piece->GetGUID()] = getMSTime();

    }

    static bool HasPendingChessMove(ObjectGuid const& pieceGuid, time_t now)
    {
        auto it = chessPendingMoveByPiece.find(pieceGuid);
        return it != chessPendingMoveByPiece.end() && now < it->second.expiresAt;
    }

    static bool IsChessDestinationReserved(uint32 instanceId, ChessSquare const& square, time_t now,
        ObjectGuid const& movingPieceGuid, ObjectGuid& reservedBy)
    {
        reservedBy = ObjectGuid::Empty;
        for (auto const& entry : chessPendingMoveByPiece)
        {
            ChessPendingMove const& pending = entry.second;
            if (entry.first == movingPieceGuid || pending.instanceId != instanceId || now >= pending.expiresAt)
                continue;
            if (pending.toSquare.row != square.row || pending.toSquare.col != square.col)
                continue;

            reservedBy = entry.first;
            return true;
        }

        return false;
    }

    static void BackoffChessMoveTrigger(ObjectGuid const& pieceGuid, ObjectGuid const& triggerGuid,
        time_t now, time_t seconds = 10)
    {
        if (pieceGuid && triggerGuid)
            chessFailedMoveTriggerUntilByPiece[pieceGuid][triggerGuid] = now + seconds;
    }

    static bool IsChessMoveTriggerBackedOff(ObjectGuid const& pieceGuid, ObjectGuid const& triggerGuid, time_t now)
    {
        auto pieceIt = chessFailedMoveTriggerUntilByPiece.find(pieceGuid);
        if (pieceIt == chessFailedMoveTriggerUntilByPiece.end())
            return false;

        auto triggerIt = pieceIt->second.find(triggerGuid);
        if (triggerIt == pieceIt->second.end())
            return false;
        if (now < triggerIt->second)
            return true;

        pieceIt->second.erase(triggerIt);
        if (pieceIt->second.empty())
            chessFailedMoveTriggerUntilByPiece.erase(pieceIt);
        return false;
    }

    static ChessPendingMoveResult TryResolvePendingChessMove(
        PlayerbotAI* botAI, Player* bot, Creature* piece, ChessBoardState const& board, time_t now, std::string& confirmSource)
    {
        confirmSource.clear();
        if (!piece)
            return ChessPendingMoveResult::NONE;

        auto it = chessPendingMoveByPiece.find(piece->GetGUID());
        if (it == chessPendingMoveByPiece.end())
            return ChessPendingMoveResult::NONE;

        ChessPendingMove const pending = it->second;
        if (now >= pending.expiresAt)
        {
            chessPendingMoveByPiece.erase(it);
            chessLastFailedMoveTriggerByPiece[piece->GetGUID()] = pending.triggerGuid;
            chessLastFailedMoveTimeByPiece[piece->GetGUID()] = now;
            BackoffChessMoveTrigger(piece->GetGUID(), pending.triggerGuid, now);
            chessOpeningMoveRetryUntilByPiece[piece->GetGUID()] = now + 5;
            auto squareIt = board.pieceSquare.find(piece->GetGUID());
            std::string const actualSquare = squareIt != board.pieceSquare.end() ?
                ChessSquareToString(squareIt->second) : "unknown";
            LogChessDecision(bot, piece, "move-expired", "from=" + ChessSquareToString(pending.fromSquare) +
                " requested=" + ChessSquareToString(pending.toSquare) + " actual=" + actualSquare +
                " source=" + pending.source + " reason=destination_not_reached", true);
            if (pending.tacticalTargetGuid)
                RegisterChessTargetFailure(bot, piece, pending.tacticalTargetGuid, now,
                    "move_expired_" + pending.source);
            return ChessPendingMoveResult::EXPIRED;
        }

        auto sqIt = board.pieceSquare.find(piece->GetGUID());
        bool const squareMatch = sqIt != board.pieceSquare.end() &&
                                 sqIt->second.row == pending.toSquare.row &&
                                 sqIt->second.col == pending.toSquare.col;
        Creature* expectedTrigger = nullptr;
        auto trigIt = board.squareToTrigger.find({ pending.toSquare.row, pending.toSquare.col });
        if (trigIt != board.squareToTrigger.end())
            expectedTrigger = trigIt->second;
        bool const triggerProximity = expectedTrigger && piece->GetExactDist2d(expectedTrigger) <= 3.5f;

        if (!squareMatch && !triggerProximity)
        {
            bool const pointMovementActive = piece->GetMotionMaster()->GetCurrentMovementGeneratorType() ==
                POINT_MOTION_TYPE;
            if (now <= pending.createdAt || pointMovementActive)
                return ChessPendingMoveResult::WAITING;

            chessPendingMoveByPiece.erase(it);
            chessLastFailedMoveTriggerByPiece[piece->GetGUID()] = pending.triggerGuid;
            chessLastFailedMoveTimeByPiece[piece->GetGUID()] = now;
            BackoffChessMoveTrigger(piece->GetGUID(), pending.triggerGuid, now);
            chessOpeningMoveRetryUntilByPiece[piece->GetGUID()] = now + 3;
            LogChessDecision(bot, piece, "move-rejected", "from=" + ChessSquareToString(pending.fromSquare) +
                " requested=" + ChessSquareToString(pending.toSquare) + " source=" + pending.source +
                " reason=no_point_movement", true);
            if (pending.tacticalTargetGuid)
                RegisterChessTargetFailure(bot, piece, pending.tacticalTargetGuid, now,
                    "move_rejected_" + pending.source);
            return ChessPendingMoveResult::EXPIRED;
        }

        confirmSource = squareMatch ? "square_match" : "trigger_proximity";
        chessPendingMoveByPiece.erase(it);
        chessOpeningMoveRetryUntilByPiece.erase(piece->GetGUID());
        chessLastFailedMoveTriggerByPiece.erase(piece->GetGUID());
        chessLastFailedMoveTimeByPiece.erase(piece->GetGUID());
        chessFailedMoveTriggerUntilByPiece.erase(piece->GetGUID());
        chessLastMoveCommandByPiece[piece->GetGUID()] = now;
        chessMovementCooldownUntilByPiece[piece->GetGUID()] = now + 5;
        chessLastMoveFromSquareByPiece[piece->GetGUID()] = pending.fromSquare;
        chessLastMoveToSquareByPiece[piece->GetGUID()] = pending.toSquare;
        chessLastSquareByPiece[piece->GetGUID()] = pending.toSquare;
        chessTargetFailureByPiece.erase(piece->GetGUID());
        LogChessDecision(bot, piece, "move-confirmed", "from=" + ChessSquareToString(pending.fromSquare) +
            " to=" + ChessSquareToString(pending.toSquare) + " source=" + pending.source +
            " confirmation=" + confirmSource, true);
        if (IsPawnEntry(piece->GetEntry()))
        {
            chessOpenedLanePawnsByInstance[pending.instanceId].insert(piece->GetGUID());
            uint32 friendlyPawns = 0;
            for (auto const& pieceEntry : board.pieceSquare)
            {
                Creature* candidate = botAI->GetCreature(pieceEntry.first);
                if (candidate && candidate->IsAlive() && IsPawnEntry(candidate->GetEntry()) &&
                    IsFriendlyChessPieceForBot(bot, candidate))
                {
                    ++friendlyPawns;
                }
            }

            uint32 availableBotControllers = 1;
            if (Group* group = bot->GetGroup())
            {
                uint32 const groupMembers = group->GetMembersCount();
                availableBotControllers = groupMembers > 1 ? groupMembers - 1 : 1;
            }
            uint32 const requiredOpenLanes = std::min<uint32>(
                4, std::min(std::max<uint32>(1, friendlyPawns), availableBotControllers));
            if (chessOpenedLanePawnsByInstance[pending.instanceId].size() >= requiredOpenLanes)
            {
                MarkOpeningProgressConfirmed(
                    pending.instanceId, bot, piece, pending.fromSquare, pending.toSquare,
                    "pending-" + pending.source + "-" + confirmSource);
            }
        }
        return ChessPendingMoveResult::CONFIRMED;
    }

    bool IsChessPieceCommandGcdReady(Creature* piece, uint32 gcdMs = 1500)
    {
        if (!piece)
            return false;

        const ObjectGuid pieceGuid = piece->GetGUID();
        const uint32 nowMs = getMSTime();
        auto it = chessLastAnyCommandMsByPiece.find(pieceGuid);
        if (it != chessLastAnyCommandMsByPiece.end() && (nowMs - it->second) < gcdMs)
            return false;

        if (piece->HasUnitState(UNIT_STATE_CASTING))
            return false;

        return true;
    }

    void StampChessPieceCommandGcd(Creature* piece)
    {
        if (!piece)
            return;

        chessLastAnyCommandMsByPiece[piece->GetGUID()] = getMSTime();
    }

    bool IsChessMoveReady(Creature* piece, time_t now, uint32 moveWindowSec = 5)
    {
        if (!piece)
            return false;
        if (piece->HasAura(SPELL_MOVE_COOLDOWN))
            return false;
        auto cdIt = chessMovementCooldownUntilByPiece.find(piece->GetGUID());
        if (cdIt != chessMovementCooldownUntilByPiece.end() && now < cdIt->second)
            return false;
        auto it = chessLastMoveCommandByPiece.find(piece->GetGUID());
        return it == chessLastMoveCommandByPiece.end() || (now - it->second) >= static_cast<time_t>(moveWindowSec);
    }

    bool IsChessAbilityReady(Creature* piece, time_t now, uint32 abilityWindowSec = 5)
    {
        if (!piece)
            return false;
        auto it = chessLastAbilityCommandByPiece.find(piece->GetGUID());
        return it == chessLastAbilityCommandByPiece.end() || (now - it->second) >= static_cast<time_t>(abilityWindowSec);
    }

    uint32 GetChessAbilityWindowSec(bool controlledNonPawn)
    {
        return controlledNonPawn ? 1u : 5u;
    }

    bool IsChessMoveSpellId(uint32 spellId)
    {
        switch (spellId)
        {
            case 37146: // SPELL_MOVE_1
            case 37144: // SPELL_MOVE_2
            case 37148: // SPELL_MOVE_3
            case 37151: // SPELL_MOVE_4
            case 37152: // SPELL_MOVE_5
            case 37153: // SPELL_MOVE_6
            case SPELL_MOVE_GENERIC:
                return true;
            default:
                return false;
        }
    }

    uint32 GetChessMoveSpellForPiece(Creature* piece)
    {
        if (!piece)
            return 0;

        for (uint8 i = 0; i < MAX_CREATURE_SPELLS; ++i)
        {
            uint32 spellId = piece->m_spells[i];
            if (spellId && IsChessMoveSpellId(spellId))
                return spellId;
        }

        return 0;
    }

    bool IsPawnEntry(uint32 e) { return e == NPC_PAWN_A || e == NPC_PAWN_H; }
    bool IsKingEntry(uint32 e) { return e == NPC_KING_A || e == NPC_KING_H; }

    uint32 GetPreferredChessPieceEntry(PlayerbotAI* botAI, Player* bot, ChessSide side)
    {
        if (!botAI || !bot || side == ChessSide::UNKNOWN)
            return 0;

        bool const alliance = side == ChessSide::ALLIANCE;
        if (botAI->IsMainTank(bot) || botAI->IsTank(bot))
            return alliance ? NPC_KING_A : NPC_KING_H;
        if (botAI->IsHeal(bot))
            return alliance ? NPC_BISHOP_A : NPC_BISHOP_H;
        if (botAI->IsRanged(bot))
            return alliance ? NPC_QUEEN_A : NPC_QUEEN_H;
        if (botAI->IsMelee(bot))
            return alliance ? NPC_KNIGHT_A : NPC_KNIGHT_H;
        return 0;
    }

    std::string ChessPhaseToString(ChessPhase p)
    {
        switch (p)
        {
            case ChessPhase::OPENING: return "OPENING";
            case ChessPhase::CLAIM_HIGH_VALUE: return "CLAIM_HIGH_VALUE";
            default: return "COMBAT";
        }
    }

    static bool WorldToChessSquare(ChessBoardState const& s, float x, float y, int& row, int& col)
    {
        // Invert:
        // x = ox + row*rowStepX + col*colStepX
        // y = oy + row*rowStepY + col*colStepY
        float dx = x - s.originX;
        float dy = y - s.originY;
        float det = (s.rowStepX * s.colStepY) - (s.colStepX * s.rowStepY);
        if (std::fabs(det) < 0.0001f)
            return false;
        float rowF = (dx * s.colStepY - s.colStepX * dy) / det;
        float colF = (s.rowStepX * dy - dx * s.rowStepY) / det;
        row = static_cast<int>(std::lround(rowF));
        col = static_cast<int>(std::lround(colF));
        return true;
    }

    static Position ChessSquareToWorld(ChessBoardState const& s, int row, int col, float z)
    {
        float x = s.originX + (s.rowStepX * row) + (s.colStepX * col);
        float y = s.originY + (s.rowStepY * row) + (s.colStepY * col);
        return Position(x, y, z);
    }

    static ChessBoardState BuildChessBoardState(PlayerbotAI* botAI, Player* bot)
    {
        ChessBoardState state;
        if (!botAI || !bot || !IsChessEncounterRelevant(botAI, bot))
            return state;

        uint32 const nowMs = getMSTime();
        auto cacheIt = chessBoardCacheByBot.find(bot->GetGUID());
        if (cacheIt != chessBoardCacheByBot.end() && (nowMs - cacheIt->second.cacheStampMs) <= ChessBoardCacheTtlMs)
            return cacheIt->second;

        std::vector<Creature*> triggers = GetNearbyChessMoveTriggers(botAI, bot);
        if (triggers.empty())
        {
            state.cacheStampMs = nowMs;
            ChessBoardState& cached = chessBoardCacheByBot[bot->GetGUID()];
            cached = state;
            return cached;
        }

        for (Creature* t : triggers)
        {
            int row = -1, col = -1;
            if (!WorldToChessSquare(state, t->GetPositionX(), t->GetPositionY(), row, col))
                continue;
            std::pair<int, int> key = { row, col };
            if (row < state.minRow || row > state.maxRow || col < state.minCol || col > state.maxCol)
                continue;
            auto it = state.squareToTrigger.find(key);
            if (it == state.squareToTrigger.end())
            {
                state.squareToTrigger[key] = t;
            }
            else
            {
                Position desired = ChessSquareToWorld(state, row, col, t->GetPositionZ());
                float dOld = std::fabs(it->second->GetPositionX() - desired.GetPositionX()) +
                             std::fabs(it->second->GetPositionY() - desired.GetPositionY());
                float dNew = std::fabs(t->GetPositionX() - desired.GetPositionX()) +
                             std::fabs(t->GetPositionY() - desired.GetPositionY());
                if (dNew < dOld)
                    state.squareToTrigger[key] = t;
            }
        }

        std::vector<Creature*> pieces = GetNearbyChessPieces(botAI, bot, false);
        for (Creature* p : pieces)
        {
            int row = -1, col = -1;
            bool const mappedLive = WorldToChessSquare(state, p->GetPositionX(), p->GetPositionY(), row, col) &&
                                    row >= state.minRow && row <= state.maxRow &&
                                    col >= state.minCol && col <= state.maxCol;
            ChessSquare sq;
            std::string fallbackSource;

            if (mappedLive)
            {
                sq = ChessSquare{ row, col };
            }
            else if (TryGetCachedChessSquare(p->GetGUID(), sq, fallbackSource) &&
                     sq.row >= state.minRow && sq.row <= state.maxRow &&
                     sq.col >= state.minCol && sq.col <= state.maxCol)
            {
                row = sq.row;
                col = sq.col;
            }
            else
            {
                continue;
            }

            state.pieceSquare[p->GetGUID()] = sq;
            state.occupied.insert({ row, col });
        }

        state.cacheStampMs = nowMs;
        ChessBoardState& cached = chessBoardCacheByBot[bot->GetGUID()];
        cached = state;
        return cached;
    }

    static bool IsInsideBoard(ChessBoardState const& b, int row, int col)
    {
        return row >= b.minRow && row <= b.maxRow && col >= b.minCol && col <= b.maxCol;
    }

    static void AppendStepMoves(ChessBoardState const& b, int row, int col, int dr, int dc,
                                std::set<std::pair<int, int>>& out)
    {
        int r = row + dr;
        int c = col + dc;
        if (IsInsideBoard(b, r, c) && b.occupied.find({ r, c }) == b.occupied.end())
            out.insert({ r, c });
    }

    static std::vector<Creature*> GetLegalMoveTriggersForPiece(PlayerbotAI* botAI, Player* bot, Creature* piece,
                                                               ChessBoardState const& b, std::string& dbgLegal)
    {
        std::vector<Creature*> legal;
        if (!botAI || !bot || !piece)
            return legal;

        auto itSq = b.pieceSquare.find(piece->GetGUID());
        if (itSq == b.pieceSquare.end())
            return legal;

        ChessSquare sq = itSq->second;
        std::set<std::pair<int, int>> candidates;
        int const maxDelta = GetChessPieceMoveDelta(piece->GetEntry());

        // Karazhan only limits the row/column delta. It does not use normal chess movement paths:
        // pawns, bishops, rooks, and kings move within a one-square box; knights within two; queens within three.
        for (int dr = -maxDelta; dr <= maxDelta; ++dr)
        {
            for (int dc = -maxDelta; dc <= maxDelta; ++dc)
            {
                if (dr || dc)
                    AppendStepMoves(b, sq.row, sq.col, dr, dc, candidates);
            }
        }

        dbgLegal.clear();
        for (auto const& rc : candidates)
        {
            auto itTr = b.squareToTrigger.find(rc);
            if (itTr == b.squareToTrigger.end())
                continue;
            bool const occupied = b.occupied.find(rc) != b.occupied.end();
            if (!dbgLegal.empty())
                dbgLegal += ";";
            dbgLegal += "cand=(" + std::to_string(rc.first) + "," + std::to_string(rc.second) + ")" +
                        " occupied=" + (occupied ? std::string("1") : std::string("0")) +
                        " trigger=" + (itTr != b.squareToTrigger.end() ? std::string("1") : std::string("0"));
            legal.push_back(itTr->second);
        }

        return legal;
    }

    struct ChessPositionEvaluation
    {
        float score = -std::numeric_limits<float>::max();
        float distance = std::numeric_limits<float>::max();
        float preferredDistance = 0.0f;
        uint32 abilitiesInRange = 0;
        uint32 readyAbilitiesInRange = 0;
        uint32 additionalTargets = 0;
        uint32 nearbyEnemies = 0;
        bool intendedTargetAttackable = false;
        bool healingPosition = false;
    };

    static uint32 GetChessHealSpell(Creature* piece, bool requireReady)
    {
        if (!piece)
            return 0;

        for (uint8 i = 0; i < MAX_CREATURE_SPELLS; ++i)
        {
            uint32 const spellId = piece->m_spells[i];
            std::string rejectReason;
            if (spellId && IsChessSupportSpell(spellId, rejectReason) &&
                (!requireReady || !piece->HasSpellCooldown(spellId)))
                return spellId;
        }

        return 0;
    }

    static float GetChessDistance2d(WorldObject const* from, WorldObject const* to)
    {
        return from && to ? from->GetExactDist2d(to) : std::numeric_limits<float>::max();
    }

    static bool CanChessSpellReachFrom(Creature* piece, WorldObject const* from, Unit* target, uint32 spellId)
    {
        if (!piece || !from || !target || !spellId)
            return false;

        float const distance = GetChessDistance2d(from, target);
        float const maxRange = GetChessOffensiveSpellRange(piece, spellId);
        float const minRange = GetChessSpellMinRange(piece, spellId);
        return distance >= minRange && (maxRange <= 0.0f || distance <= maxRange);
    }

    static uint32 CountEnemiesNearPosition(
        Player* bot, ChessBoardState const& board, WorldObject const* position, float radius, Creature const* ignored = nullptr)
    {
        if (!bot || !position)
            return 0;

        uint32 count = 0;
        for (auto const& boardEntry : board.pieceSquare)
        {
            Creature* candidate = ObjectAccessor::GetCreature(*bot, boardEntry.first);
            if (!candidate || candidate == ignored || !candidate->IsAlive() || !IsEnemyChessPieceForBot(bot, candidate))
                continue;
            if (position->GetExactDist2d(candidate) <= radius)
                ++count;
        }
        return count;
    }

    static uint32 CountEnemiesInProspectiveCone(Player* bot, ChessBoardState const& board,
        WorldObject const* position, Creature* intendedTarget, float radius)
    {
        if (!bot || !position || !intendedTarget)
            return 0;

        float const targetX = intendedTarget->GetPositionX() - position->GetPositionX();
        float const targetY = intendedTarget->GetPositionY() - position->GetPositionY();
        float const targetLength = std::sqrt(targetX * targetX + targetY * targetY);
        if (targetLength < 0.01f)
            return 0;

        uint32 count = 0;
        float const minimumCosine = std::cos(ChessConeAngle * 0.5f);
        for (auto const& boardEntry : board.pieceSquare)
        {
            Creature* candidate = ObjectAccessor::GetCreature(*bot, boardEntry.first);
            if (!candidate || candidate == intendedTarget || !candidate->IsAlive() ||
                !IsEnemyChessPieceForBot(bot, candidate))
                continue;

            float const candidateX = candidate->GetPositionX() - position->GetPositionX();
            float const candidateY = candidate->GetPositionY() - position->GetPositionY();
            float const candidateLength = std::sqrt(candidateX * candidateX + candidateY * candidateY);
            if (candidateLength > radius || candidateLength < 0.01f)
                continue;

            float const cosine = (targetX * candidateX + targetY * candidateY) /
                (targetLength * candidateLength);
            if (cosine >= minimumCosine)
                ++count;
        }

        return count;
    }

    static float GetPreferredChessAttackDistance(Creature* piece)
    {
        float shortestRange = std::numeric_limits<float>::max();
        for (uint32 spellId : GetLikelyOffensiveChessSpells(piece))
        {
            float const range = GetChessOffensiveSpellRange(piece, spellId);
            if (range > 0.0f)
                shortestRange = std::min(shortestRange, range);
        }

        if (shortestRange == std::numeric_limits<float>::max())
            return ChessAutomaticMeleeRange - 2.0f;
        return std::max(2.0f, shortestRange - 2.0f);
    }

    static ChessPositionEvaluation EvaluateChessPosition(Player* bot, Creature* piece, ChessBoardState const& board,
        WorldObject const* position, Creature* tacticalTarget, bool healing, ChessSquare const& candidateSquare,
        ChessSquare const& currentSquare)
    {
        ChessPositionEvaluation evaluation;
        if (!bot || !piece || !position || !tacticalTarget)
            return evaluation;

        evaluation.distance = GetChessDistance2d(position, tacticalTarget);
        evaluation.healingPosition = healing;
        if (healing)
        {
            uint32 const healSpell = GetChessHealSpell(piece, false);
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(healSpell);
            float const maxRange = spellInfo ? spellInfo->GetMaxRange(true, piece) : 0.0f;
            float const minRange = spellInfo ? spellInfo->GetMinRange(true) : 0.0f;
            evaluation.preferredDistance = std::max(2.0f, maxRange - 3.0f);
            evaluation.intendedTargetAttackable = healSpell && evaluation.distance >= minRange &&
                (maxRange <= 0.0f || evaluation.distance <= maxRange);
            evaluation.abilitiesInRange = evaluation.intendedTargetAttackable ? 1 : 0;
            evaluation.readyAbilitiesInRange = evaluation.intendedTargetAttackable &&
                !piece->HasSpellCooldown(healSpell) ? 1 : 0;
        }
        else
        {
            evaluation.preferredDistance = GetPreferredChessAttackDistance(piece);
            for (uint32 spellId : GetLikelyOffensiveChessSpells(piece))
            {
                if (!CanChessSpellReachFrom(piece, position, tacticalTarget, spellId))
                    continue;

                ++evaluation.abilitiesInRange;
                if (!piece->HasSpellCooldown(spellId))
                    ++evaluation.readyAbilitiesInRange;

                if (IsTargetAreaOffensiveChessSpell(spellId))
                {
                    float const radius = GetChessSpellEffectRadius(piece, spellId);
                    evaluation.additionalTargets = std::max(evaluation.additionalTargets,
                        CountEnemiesNearPosition(bot, board, tacticalTarget, radius, tacticalTarget));
                }
                else if (IsConeOffensiveChessSpell(spellId))
                {
                    float const radius = GetChessSpellEffectRadius(piece, spellId);
                    evaluation.additionalTargets = std::max(evaluation.additionalTargets,
                        CountEnemiesInProspectiveCone(bot, board, position, tacticalTarget, radius));
                }
                else if (IsCasterCenteredOffensiveChessSpell(spellId))
                {
                    float const radius = GetChessSpellEffectRadius(piece, spellId);
                    evaluation.additionalTargets = std::max(evaluation.additionalTargets,
                        CountEnemiesNearPosition(bot, board, position, radius, tacticalTarget));
                }
            }

            evaluation.intendedTargetAttackable = evaluation.abilitiesInRange > 0;
            if (!evaluation.intendedTargetAttackable && evaluation.distance <= ChessAutomaticMeleeRange)
                evaluation.intendedTargetAttackable = true;
        }

        evaluation.nearbyEnemies = CountEnemiesNearPosition(bot, board, position, ChessAutomaticMeleeRange);
        evaluation.score = evaluation.intendedTargetAttackable ? 100000.0f : 0.0f;
        evaluation.score += static_cast<float>(evaluation.abilitiesInRange) * 5000.0f;
        evaluation.score += static_cast<float>(evaluation.readyAbilitiesInRange) * 2500.0f;
        evaluation.score += static_cast<float>(evaluation.additionalTargets) * 500.0f;
        evaluation.score -= std::fabs(evaluation.distance - evaluation.preferredDistance) * 150.0f;
        evaluation.score -= static_cast<float>(evaluation.nearbyEnemies) * (healing ? 450.0f : 100.0f);
        evaluation.score -= static_cast<float>(std::max(
            std::abs(candidateSquare.row - currentSquare.row),
            std::abs(candidateSquare.col - currentSquare.col))) * 25.0f;

        bool const onEdge = candidateSquare.row == board.minRow || candidateSquare.row == board.maxRow ||
            candidateSquare.col == board.minCol || candidateSquare.col == board.maxCol;
        if (onEdge)
            evaluation.score -= 75.0f;

        auto lastFrom = chessLastMoveFromSquareByPiece.find(piece->GetGUID());
        auto lastTo = chessLastMoveToSquareByPiece.find(piece->GetGUID());
        if (lastFrom != chessLastMoveFromSquareByPiece.end() && lastTo != chessLastMoveToSquareByPiece.end() &&
            lastTo->second.row == currentSquare.row && lastTo->second.col == currentSquare.col &&
            lastFrom->second.row == candidateSquare.row && lastFrom->second.col == candidateSquare.col)
            evaluation.score -= 20000.0f;

        return evaluation;
    }

    static Creature* GetChessFacingTrigger(
        ChessBoardState const& board, ChessSquare const& from, ChessSquare const& target)
    {
        int const rowDelta = std::clamp(target.row - from.row, -3, 3);
        int const colDelta = std::clamp(target.col - from.col, -3, 3);
        if (!rowDelta && !colDelta)
            return nullptr;

        auto trigger = board.squareToTrigger.find({ from.row + rowDelta, from.col + colDelta });
        return trigger != board.squareToTrigger.end() ? trigger->second : nullptr;
    }

    static bool TryFaceChessTarget(Player* bot, Creature* piece, ChessBoardState const& board,
        ChessSquare const& currentSquare, Creature* target, time_t now, std::string const& reason)
    {
        if (!bot || !piece || !target || !IsChessMoveReady(piece, now, 5) || piece->HasSpellCooldown(SpellChangeFacing))
            return false;

        auto targetSquare = board.pieceSquare.find(target->GetGUID());
        if (targetSquare == board.pieceSquare.end())
            return false;

        Creature* trigger = GetChessFacingTrigger(board, currentSquare, targetSquare->second);
        if (!trigger || !CastChessSpell(piece, trigger, SpellChangeFacing))
        {
            LogChessDecision(bot, piece, "face-failed", "target=" + target->GetName() + " reason=" + reason);
            return false;
        }

        StampChessPieceCommandGcd(piece);
        chessLastMoveCommandByPiece[piece->GetGUID()] = now;
        chessMovementCooldownUntilByPiece[piece->GetGUID()] = now + 5;
        LogChessDecision(bot, piece, "face-command", "target=" + target->GetName() + " piece_square=" +
            ChessSquareToString(currentSquare) + " target_square=" + ChessSquareToString(targetSquare->second) +
            " reason=" + reason, true);
        return true;
    }

    static ChessPhase GetChessPhase(PlayerbotAI* botAI, Player* bot, ChessBoardState const& b)
    {
        if (!bot || !bot->GetMap())
            return ChessPhase::COMBAT;

        const uint32 instanceId = bot->GetMap()->GetInstanceId();
        if (HasConfirmedOpeningProgress(instanceId))
        {
            return ChessPhase::CLAIM_HIGH_VALUE;
        }
        uint32 movedPawns = 0;
        uint32 friendlyPawns = 0;
        for (auto const& pieceEntry : b.pieceSquare)
        {
            ObjectGuid const& pieceGuid = pieceEntry.first;
            Creature* p = botAI->GetCreature(pieceGuid);
            if (!p || !p->IsAlive() || !IsFriendlyChessPieceForBot(bot, p))
                continue;
            if (IsPawnEntry(p->GetEntry()))
            {
                ++friendlyPawns;
                if (chessOpenedLanePawnsByInstance[instanceId].find(pieceGuid) != chessOpenedLanePawnsByInstance[instanceId].end())
                    ++movedPawns;
                continue;
            }
        }

        const bool openingDoneByPawns = friendlyPawns == 0 || movedPawns >= std::min<uint32>(4, friendlyPawns);
        if (!openingDoneByPawns)
            return ChessPhase::OPENING;

        // Short transition phase to prioritize king/healer/ranged claims once lanes are open.
        const time_t now = std::time(nullptr);
        auto itStart = chessEventStartByInstance.find(instanceId);
        if (itStart != chessEventStartByInstance.end() && (now - itStart->second) < 45)
            return ChessPhase::CLAIM_HIGH_VALUE;

        return ChessPhase::COMBAT;
    }

    static bool HasAvailableNonPawnChessPiece(PlayerbotAI* botAI, Player* bot, bool openingProgressConfirmed)
    {
        if (!botAI || !bot || !openingProgressConfirmed)
            return false;

        bool const canClaimKing = botAI->IsMainTank(bot) || botAI->IsTank(bot);
        for (Creature* candidate : GetNearbyChessPieces(botAI, bot, true))
        {
            if (!candidate || IsPawnEntry(candidate->GetEntry()) ||
                (IsKingEntry(candidate->GetEntry()) && !canClaimKing) ||
                IsPieceAssignedToOtherBot(bot, candidate))
                continue;
            if (IsClaimableChessPieceForBot(bot, candidate, openingProgressConfirmed))
                return true;
        }

        return false;
    }
}

bool KarazhanChessPassiveHelperAction::Execute(Event /*event*/)
{
    if (!IsChessEncounterRelevant(botAI, bot))
        return false;

    if (!IsChessEventActive(botAI, bot))
        return false;

    Creature* piece = bot->GetCharm() ? bot->GetCharm()->ToCreature() : nullptr;
    if (!piece)
        piece = GetAssignedChessPiece(bot);

    if (piece)
    {
        Unit* target = botAI->GetUnit(bot->GetTarget());
        if (target && IsChessPieceEntry(target->GetEntry()))
            bot->SetTarget(ObjectGuid::Empty);

        if (bot->IsInCombat())
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
        }
    }

    return false;
}

bool KarazhanChessClaimPieceAction::Execute(Event /*event*/)
{
    if (!IsChessEncounterRelevant(botAI, bot))
        return false;

    if (!IsChessEventActive(botAI, bot))
        return false;

    const uint32 instanceId = bot->GetMap() ? bot->GetMap()->GetInstanceId() : 0;
    const time_t now = std::time(nullptr);
    if (instanceId)
        chessEventStartByInstance.try_emplace(instanceId, now);
    CleanupStaleChessState(botAI, bot, "claim-piece");
    ChessBoardState board = BuildChessBoardState(botAI, bot);
    ChessPhase phase = GetChessPhase(botAI, bot, board);
    const bool canClaimKing = botAI->IsMainTank(bot) || botAI->IsTank(bot);

    ChessSide botSide = GetChessSideForBot(bot);
    if (botSide == ChessSide::UNKNOWN)
    {
        return false;
    }

    bool const openingProgressConfirmed = HasConfirmedOpeningProgress(instanceId);
    bool const pawnOnlyMode = !openingProgressConfirmed;

    Creature* charm = bot->GetCharm() ? bot->GetCharm()->ToCreature() : nullptr;
    if (charm && IsChessPieceEntry(charm->GetEntry()))
    {
        ClearPendingChessClaim(bot, "already-charmed-chess-piece");
        return false;
    }

    if (openingProgressConfirmed)
    {
        ChessPendingClaim* pending = GetPendingChessClaim(bot);
        if (pending && pending->expiresAt <= now)
        {
            ObjectGuid const expiredPieceGuid = pending->pieceGuid;
            ClearPendingChessClaim(bot, "timeout");
            if (Creature* assignedPiece = GetAssignedChessPiece(bot))
            {
                if (assignedPiece->GetGUID() == expiredPieceGuid)
                {
                    SuppressReclaimForPiece(expiredPieceGuid, now, 15);
                    ClearAssignedChessPiece(bot);
                }
            }
            pending = nullptr;
        }

        if (pending)
        {
            Creature* pendingPiece = botAI->GetCreature(pending->pieceGuid);
            bool const pendingValid = pendingPiece && pendingPiece->IsInWorld() && pendingPiece->IsAlive() && IsChessPieceEntry(pendingPiece->GetEntry()) &&
                IsFriendlyChessPieceForBot(bot, pendingPiece) && !IsPieceAssignedToOtherBot(bot, pendingPiece);

            if (!pendingValid)
            {
                ClearPendingChessClaim(bot, "target-invalid");
                if (pendingPiece && pendingPiece->IsAlive() && IsChessPieceEntry(pendingPiece->GetEntry()))
                    SuppressReclaimForPiece(pendingPiece->GetGUID(), now, 20);
            }
            else
            {
                Creature* assignedPending = GetAssignedChessPiece(bot);
                if (!assignedPending || assignedPending->GetGUID() != pendingPiece->GetGUID())
                    SetAssignedChessPiece(bot, pendingPiece, 30);

                float const dist = bot->GetExactDist2d(pendingPiece);

                if (dist > 8.0f)
                {
                    return MoveTo(KARAZHAN_MAP_ID, pendingPiece->GetPositionX(), pendingPiece->GetPositionY(), pendingPiece->GetPositionZ(),
                                  false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
                }

                TryPendingChessClaimControl(bot, pendingPiece, *pending, now);

                return true;
            }
        }
    }

    Creature* assigned = GetAssignedChessPiece(bot);
    if (assigned && assigned->IsAlive())
    {
        if (pawnOnlyMode && !IsPawnEntry(assigned->GetEntry()))
        {
            ClearPendingChessClaim(bot, "pawn-only-mode");
            ClearAssignedChessPiece(bot);
            return false;
        }

        if (!pawnOnlyMode && !IsClaimableChessPieceForBot(bot, assigned, openingProgressConfirmed))
        {
            ClearPendingChessClaim(bot, "not-claimable-after-opening");
            ClearAssignedChessPiece(bot);
            return false;
        }

        if (IsKingEntry(assigned->GetEntry()) && !canClaimKing)
        {
            ClearPendingChessClaim(bot, "king-restricted");
            ClearAssignedChessPiece(bot);
            return false;
        }

        if (pawnOnlyMode && !IsClaimableFriendlyPawnForOpening(bot, assigned))
        {
            ClearPendingChessClaim(bot, "opening-not-pawn");
            ClearAssignedChessPiece(bot);
            return false;
        }
        float dist = bot->GetExactDist2d(assigned);
        if (openingProgressConfirmed)
        {
            ChessPendingClaim* pending = GetPendingChessClaim(bot);
            if (!pending || pending->pieceGuid != assigned->GetGUID())
                SetPendingChessClaim(bot, assigned, instanceId, now, 30);
        }
        if (dist > 8.0f)
        {
            return MoveTo(KARAZHAN_MAP_ID, assigned->GetPositionX(), assigned->GetPositionY(), assigned->GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
        }
        if (ChessPendingClaim* pending = GetPendingChessClaim(bot))
        {
            if (pending->pieceGuid == assigned->GetGUID())
                TryPendingChessClaimControl(bot, assigned, *pending, now);
        }
        else
            bot->CastSpell(assigned, SPELL_CONTROL_PIECE, true);
        return true;
    }

    std::vector<Creature*> nearby = GetNearbyChessPieces(botAI, bot, false);
    if (nearby.empty())
    {
        return false;
    }

    Creature* bestPreferred = nullptr;
    Creature* bestFallback = nullptr;
    uint32 bestPreferredScore = 0;
    uint32 bestFallbackScore = 0;
    uint32 const preferredEntry = pawnOnlyMode ? 0 : GetPreferredChessPieceEntry(botAI, bot, botSide);
    bool hasClaimablePawn = false;
    for (Creature* p : nearby)
    {
        if (!p || !IsClaimableFriendlyPawnForOpening(bot, p) || IsPieceAssignedToOtherBot(bot, p))
            continue;
        hasClaimablePawn = true;
        break;
    }

    for (Creature* piece : nearby)
    {
        if (!piece)
            continue;

        bool const isPawn = IsPawnEntry(piece->GetEntry());
        bool finalClaimAllowed = false;
        std::string rejectReason;
        ChessSide const pieceBotSide = GetChessSideForBot(bot);
        bool const sideKnown = pieceBotSide != ChessSide::UNKNOWN;
        bool const isFriendlyForBot = IsFriendlyChessPieceForBot(bot, piece);
        bool const alive = piece->IsAlive();
        bool const charmed = piece->IsCharmed();
        bool const notSelectable = piece->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        bool const assignedToOtherBot = IsPieceAssignedToOtherBot(bot, piece);
        std::string reclaimReason;
        bool const reclaimSuppressed = IsReclaimSuppressed(piece->GetGUID(), now, reclaimReason);
        bool const openingClaimable = pawnOnlyMode ? IsClaimableFriendlyPawnForOpening(bot, piece) : IsClaimableChessPieceForBot(bot, piece, openingProgressConfirmed);

        if (reclaimSuppressed)
            rejectReason = reclaimReason;
        else if (!openingClaimable)
        {
            if (!sideKnown)
                rejectReason = "side-unknown";
            else if (!alive)
                rejectReason = "dead";
            else if (!isFriendlyForBot)
                rejectReason = "wrong-side";
            else if (assignedToOtherBot)
                rejectReason = "assigned-to-other-bot";
            else if (!openingProgressConfirmed && !isPawn)
                rejectReason = "not-pawn";
            else if (!openingProgressConfirmed && charmed)
                rejectReason = "already-charmed";
            else if (!openingProgressConfirmed && notSelectable)
                rejectReason = "not-selectable";
            else
                rejectReason = "not-claimable";
        }
        else if (IsKingEntry(piece->GetEntry()) && !canClaimKing)
            rejectReason = "king-restricted-to-tank";
        else if (assignedToOtherBot)
            rejectReason = "assigned-to-other-bot";
        else if (pawnOnlyMode && !isPawn)
            rejectReason = "pawn-only-mode";
        else if (phase == ChessPhase::OPENING && hasClaimablePawn && !isPawn)
            rejectReason = "opening-pawn-phase";
        else
            finalClaimAllowed = true;

        if (!finalClaimAllowed)
            continue;

        // Hard guard requested: never select non-claimable targets.
        if (pawnOnlyMode && !IsClaimableFriendlyPawnForOpening(bot, piece))
        {
            continue;
        }

        uint32 score = 10;
        switch (piece->GetEntry())
        {
            case NPC_KING_A:
            case NPC_KING_H:
                score += botAI->IsTank(bot) ? 260 : (botAI->IsHeal(bot) ? 120 : 40);
                break;
            case NPC_BISHOP_A:
            case NPC_BISHOP_H:
                score += botAI->IsHeal(bot) ? 260 : (botAI->IsRanged(bot) ? 120 : 40);
                break;
            case NPC_QUEEN_A:
            case NPC_QUEEN_H:
                score += botAI->IsRanged(bot) ? 220 : 80;
                break;
            case NPC_ROOK_A:
            case NPC_ROOK_H:
                score += (botAI->IsRanged(bot) || botAI->IsTank(bot)) ? 180 : 80;
                break;
            case NPC_KNIGHT_A:
            case NPC_KNIGHT_H:
                score += botAI->IsMelee(bot) ? 180 : 90;
                break;
            case NPC_PAWN_A:
            case NPC_PAWN_H:
                score += botAI->IsMelee(bot) ? 130 : 70;
                break;
            default:
                score += 40;
                break;
        }

        if (piece->GetHealthPct() < 50.0f)
            score -= 30;

        if (openingProgressConfirmed && isPawn)
        {
            // If a bot must fall back to a pawn, advance an untouched lane before reclaiming one that already
            // opened the back rank. This progressively creates more exits without delaying the four-pawn opener.
            score += HasPawnMovedDuringOpening(instanceId, piece->GetGUID()) ? 0 : 180;
        }

        if (phase == ChessPhase::OPENING)
        {
            if (IsPawnEntry(piece->GetEntry()))
            {
                score += 1200;
                auto squareIt = board.pieceSquare.find(piece->GetGUID());
                if (squareIt != board.pieceSquare.end() && IsOpeningChessPawnFile(squareIt->second))
                    score += 500;
            }
            else
                score -= 500;
        }
        else if (phase == ChessPhase::CLAIM_HIGH_VALUE)
        {
            if (IsKingEntry(piece->GetEntry()))
                score += 1000;
            else if (IsHealerChessPieceEntry(piece->GetEntry()))
                score += 700;
            else if (IsDamageChessPieceEntry(piece->GetEntry()))
                score += 500;
            else if (piece->GetEntry() == NPC_KNIGHT_A || piece->GetEntry() == NPC_KNIGHT_H)
                score += 350;
        }

        bool const preferred = preferredEntry && piece->GetEntry() == preferredEntry;
        Creature*& bestForPass = preferred ? bestPreferred : bestFallback;
        uint32& bestScoreForPass = preferred ? bestPreferredScore : bestFallbackScore;
        if (!bestForPass || score > bestScoreForPass)
        {
            bestForPass = piece;
            bestScoreForPass = score;
        }
    }

    Creature* best = bestPreferred ? bestPreferred : bestFallback;

    if (!best || !SetAssignedChessPiece(bot, best, 10))
    {
        return false;
    }

    if (openingProgressConfirmed)
        SetPendingChessClaim(bot, best, instanceId, now, 30);

    LogChessDecision(bot, best, "piece-claim", "phase=" + ChessPhaseToString(phase) + " preferred_entry=" +
        std::to_string(preferredEntry) + " selected_entry=" + std::to_string(best->GetEntry()) + " selection=" +
        (bestPreferred ? "preferred" : "fallback"), true);

    float bestDist = bot->GetExactDist2d(best);
    if (bestDist > 8.0f)
    {
        return MoveTo(KARAZHAN_MAP_ID, best->GetPositionX(), best->GetPositionY(), best->GetPositionZ(),
                      false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    if (ChessPendingClaim* pending = GetPendingChessClaim(bot))
    {
        if (pending->pieceGuid == best->GetGUID())
            TryPendingChessClaimControl(bot, best, *pending, now);
    }
    else
        bot->CastSpell(best, SPELL_CONTROL_PIECE, true);
    return true;
}

bool KarazhanChessMovePieceAction::Execute(Event /*event*/)
{
    Creature* piece = bot->GetCharm() ? bot->GetCharm()->ToCreature() : nullptr;
    if (!piece || !IsChessPieceEntry(piece->GetEntry()))
        return false;
    if (!IsChessEncounterRelevant(botAI, bot))
        return false;
    if (!IsChessPieceCommandGcdReady(piece))
    {
        return false;
    }
    const time_t now = std::time(nullptr);
    const uint32 instanceId = bot->GetMap() ? bot->GetMap()->GetInstanceId() : 0;
    bool const openingProgressConfirmed = HasConfirmedOpeningProgress(instanceId);
    bool const controlledNonPawn = !IsPawnEntry(piece->GetEntry()) && openingProgressConfirmed;
    if (!IsPawnEntry(piece->GetEntry()) && !controlledNonPawn)
    {
        return false;
    }
    if (controlledNonPawn)
    {
    }
    ChessBoardState board = BuildChessBoardState(botAI, bot);
    ChessSquare controlledSq;
    std::string controlledRejectReason;
    if (!IsActiveBoardFriendlyControlledChessPiece(bot, board, piece, controlledSq, controlledRejectReason))
    {
        HandleInvalidControlledChessPiece(botAI, bot, piece, board, "move_piece", controlledRejectReason);
        return false;
    }
    auto pieceSqIt = board.pieceSquare.find(piece->GetGUID());
    if (pieceSqIt != board.pieceSquare.end())
        chessLastSquareByPiece[piece->GetGUID()] = pieceSqIt->second;
    time_t const nowTick = now;
    std::string confirmSource;
    ChessPendingMoveResult const pendingResult = TryResolvePendingChessMove(botAI, bot, piece, board, now, confirmSource);
    if (pendingResult == ChessPendingMoveResult::CONFIRMED || pendingResult == ChessPendingMoveResult::WAITING || pendingResult == ChessPendingMoveResult::EXPIRED)
        return true;

    if (!IsChessMoveReady(piece, now, 5))
    {
        return false;
    }

    Creature* enemyKing = GetEnemyChessKing(botAI, bot);
    if (!enemyKing)
    {
        return false;
    }

    ChessPhase phase = GetChessPhase(botAI, bot, board);
    bool const pawnMovedThisOpening = HasPawnMovedDuringOpening(instanceId, piece->GetGUID());
    auto kingSqIt = board.pieceSquare.find(enemyKing->GetGUID());
    if (kingSqIt != board.pieceSquare.end())
    {
        ChessSquare current = kingSqIt->second;
        auto prevIt = chessLastEnemyKingSquareByBot.find(bot->GetGUID());
        if (prevIt == chessLastEnemyKingSquareByBot.end() ||
            prevIt->second.row != current.row || prevIt->second.col != current.col)
        {
            std::string prev = (prevIt == chessLastEnemyKingSquareByBot.end()) ? "none" :
                ("(" + std::to_string(prevIt->second.row) + "," + std::to_string(prevIt->second.col) + ")");
            chessLastEnemyKingSquareByBot[bot->GetGUID()] = current;
        }
    }
    else
    {
    }

    if (phase == ChessPhase::OPENING && pawnMovedThisOpening)
    {
        return false;
    }

    auto retryIt = chessOpeningMoveRetryUntilByPiece.find(piece->GetGUID());
    if (phase == ChessPhase::OPENING && retryIt != chessOpeningMoveRetryUntilByPiece.end() && now < retryIt->second)
    {
        return false;
    }

    std::string legalDbg;
    std::vector<Creature*> triggers = GetLegalMoveTriggersForPiece(botAI, bot, piece, board, legalDbg);
    uint32 const moveSpell = GetChessMoveSpellForPiece(piece);
    std::vector<std::pair<float, Creature*>> legalMoves;
    ObjectGuid const pieceGuid = piece->GetGUID();
    auto moveIt = chessLastMoveCommandByPiece.find(pieceGuid);
    if (moveIt != chessLastMoveCommandByPiece.end() && (nowTick - moveIt->second) < 2)
    {
        return false;
    }

    bool sawReservedDestination = false;
    bool sawFailedDestinationBackoff = false;

    for (Creature* trigger : triggers)
    {
        float distPiece = piece->GetExactDist2d(trigger);
        float distKing = trigger->GetExactDist2d(enemyKing);
        if (distPiece > 30.0f)
            continue;

        // Give the event board time to settle and force an alternate square after a rejected move.
        if (IsChessMoveTriggerBackedOff(pieceGuid, trigger->GetGUID(), nowTick))
        {
            sawFailedDestinationBackoff = true;
            continue;
        }

        int reservedRow = -1;
        int reservedCol = -1;
        ObjectGuid reservedBy;
        if (WorldToChessSquare(board, trigger->GetPositionX(), trigger->GetPositionY(), reservedRow, reservedCol) &&
            IsChessDestinationReserved(instanceId, ChessSquare{ reservedRow, reservedCol }, nowTick,
                pieceGuid, reservedBy))
        {
            sawReservedDestination = true;
            continue;
        }

        legalMoves.push_back({ distKing, trigger });
    }

    if (legalMoves.empty())
    {
        bool const hasMoveSpell = moveSpell != 0;

        std::string curSq = "unknown";
        if (pieceSqIt != board.pieceSquare.end())
            curSq = "(" + std::to_string(pieceSqIt->second.row) + "," + std::to_string(pieceSqIt->second.col) + ")";

        std::string squareBounds = "rows=" + std::to_string(board.minRow) + ".." + std::to_string(board.maxRow) +
                                   " cols=" + std::to_string(board.minCol) + ".." + std::to_string(board.maxCol);
        auto targetIt = chessOffensiveTargetByPiece.find(piece->GetGUID());
        if (!sawReservedDestination && !sawFailedDestinationBackoff &&
            targetIt != chessOffensiveTargetByPiece.end())
            RegisterChessTargetFailure(bot, piece, targetIt->second, now, "no_legal_move_square");
        std::string const reason = sawReservedDestination ? "destination_reserved" :
            (sawFailedDestinationBackoff ? "failed_destination_backoff" : "no_legal_move_square");
        LogChessDecision(bot, piece, "move-wait", "reason=" + reason + " current_square=" + curSq +
            " board=" + squareBounds + " has_move_spell=" + (hasMoveSpell ? "1" : "0"));
        return false;
    }

    std::sort(legalMoves.begin(), legalMoves.end(),
        [](std::pair<float, Creature*> const& lhs, std::pair<float, Creature*> const& rhs)
        {
            return lhs.first < rhs.first;
        });

    // Pawn-only deterministic opener: one legal forward/outward pawn move via native chess move spell.
    // Post-opening controlled non-pawns reuse the same native chess move handling.
    if (IsPawnEntry(piece->GetEntry()) || controlledNonPawn)
    {
        if (pieceSqIt == board.pieceSquare.end())
        {
            ChessSquare cached;
            std::string source;
            if (TryGetCachedChessSquare(piece->GetGUID(), cached, source))
            {
                pieceSqIt = board.pieceSquare.emplace(piece->GetGUID(), cached).first;
                chessLastSquareByPiece[piece->GetGUID()] = cached;
            }

            if (pieceSqIt == board.pieceSquare.end())
            {
                return false;
            }
        }
        ChessSquare curSq = pieceSqIt->second;
        bool const pawnOpening = IsPawnEntry(piece->GetEntry()) && phase == ChessPhase::OPENING;
        int pawnOpeningForward = 0;
        int pawnOpeningPreferredCol = curSq.col;
        if (pawnOpening)
        {
            if (!IsOpeningChessPawnFile(curSq) || kingSqIt == board.pieceSquare.end())
                return false;

            pawnOpeningForward = kingSqIt->second.row > curSq.row ? 1 : -1;
            pawnOpeningPreferredCol += curSq.col <= 2 ? 1 : -1;
        }
        int bestRow = std::numeric_limits<int>::max();
        int bestCol = std::numeric_limits<int>::max();
        float bestMoveScore = -std::numeric_limits<float>::max();
        ChessPositionEvaluation bestEvaluation;
        Creature* bestForwardTrigger = nullptr;
        Creature* moveTarget = nullptr;
        std::string moveTargetReason = "enemy_king";
        ChessPositionEvaluation currentEvaluation;
        bool tacticalHealing = false;
        bool const targetDrivenMovement = controlledNonPawn || !pawnOpening;
        if (targetDrivenMovement)
        {
            ChessOffensiveTargetSelection movementTarget;
            if (IsHealerChessPieceEntry(piece->GetEntry()) && GetChessHealSpell(piece, true))
            {
                ChessSupportTargetSelection healingTarget = SelectDamagedFriendlyActiveBoardTarget(bot, board, piece);
                if (healingTarget.target)
                {
                    moveTarget = healingTarget.target;
                    moveTargetReason = "damaged_friendly";
                    tacticalHealing = true;
                }
            }

            if (!moveTarget)
            {
                movementTarget = SelectPersistentChessTarget(botAI, bot, piece, board, "movement");
                if (movementTarget.target)
                {
                    moveTarget = movementTarget.target;
                    moveTargetReason = "selected_nonking_" + movementTarget.category;
                }
                else if (enemyKing)
                {
                    uint32 supportAlive = 0;
                    uint32 damageAlive = 0;
                    uint32 pawnAlive = 0;
                    std::string gateReason = "no_enemy_king";
                    bool const kingMovementAllowed = IsKarazhanChessKingFocusAllowedActiveBoard(
                        bot, board, enemyKing, supportAlive, damageAlive, pawnAlive, gateReason);
                    bool const noActionableMovementTarget =
                        IsNoActionableNonKingRejectReason(movementTarget.rejectReason);
                    bool const movementDeadlockBreaker =
                        !kingMovementAllowed &&
                        noActionableMovementTarget &&
                        gateReason == "board_not_cleared" &&
                        damageAlive <= 1 &&
                        !GetLikelyOffensiveChessSpells(piece).empty();
                    if (movementDeadlockBreaker)
                    {
                    }
                    else
                    {
                        std::string kingFallbackRejectedReason = "none";
                        if (!noActionableMovementTarget)
                            kingFallbackRejectedReason = "target_actionable";
                        else if (gateReason != "board_not_cleared")
                            kingFallbackRejectedReason = "gate_" + gateReason;
                        else if (damageAlive > 1)
                            kingFallbackRejectedReason = "enemy_damage_alive_gt_1";
                        else if (GetLikelyOffensiveChessSpells(piece).empty())
                            kingFallbackRejectedReason = "no_offensive_spells";
                    }

                    if (kingMovementAllowed || movementDeadlockBreaker)
                    {
                        moveTarget = enemyKing;
                        moveTargetReason = movementDeadlockBreaker ?
                            "enemy_king_fallback_breaker" : "enemy_king_fallback";
                    }
                    else
                    {
                        return false;
                    }
                }

            }

            if (!moveTarget)
                return false;

            currentEvaluation = EvaluateChessPosition(
                bot, piece, board, piece, moveTarget, tacticalHealing, curSq, curSq);
            if (currentEvaluation.intendedTargetAttackable)
            {
                if (!tacticalHealing)
                {
                    bool coneInRange = false;
                    for (uint32 spellId : GetLikelyOffensiveChessSpells(piece))
                    {
                        if (IsConeOffensiveChessSpell(spellId) &&
                            CanChessSpellReachFrom(piece, piece, moveTarget, spellId))
                        {
                            coneInRange = true;
                            break;
                        }
                    }
                    float const facingArc = coneInRange ? ChessConeAngle : static_cast<float>(M_PI);
                    bool const needsFacing = coneInRange ||
                        piece->GetExactDist2d(moveTarget) <= ChessAutomaticMeleeRange;
                    if (needsFacing && !piece->HasInArc(facingArc, moveTarget))
                        return TryFaceChessTarget(bot, piece, board, curSq, moveTarget, now, moveTargetReason);
                }

                bool const stableRange = tacticalHealing || currentEvaluation.preferredDistance <= 10.0f ||
                    currentEvaluation.distance >= currentEvaluation.preferredDistance * 0.65f;
                if (stableRange)
                {
                    LogChessDecision(bot, piece, "hold-position", "target=" + moveTarget->GetName() +
                        " piece_square=" + ChessSquareToString(curSq) + " target_distance=" +
                        std::to_string(currentEvaluation.distance) + " preferred_distance=" +
                        std::to_string(currentEvaluation.preferredDistance) + " abilities_in_range=" +
                        std::to_string(currentEvaluation.abilitiesInRange) + " reason=" + moveTargetReason);
                    return false;
                }
            }
        }
        std::string candDbg;
        for (std::pair<float, Creature*> const& mv : legalMoves)
        {
            int row = -1, col = -1;
            bool const mapped = WorldToChessSquare(board, mv.second->GetPositionX(), mv.second->GetPositionY(), row, col);
            bool const inBounds = mapped && IsInsideBoard(board, row, col);
            Position candWorld = ChessSquareToWorld(board, row, col, mv.second->GetPositionZ());
            bool const occupied = inBounds && board.occupied.find({ row, col }) != board.occupied.end();
            std::string occBy = "none";
            if (occupied)
            {
                for (auto const& pg : board.pieceSquare)
                {
                    if (pg.second.row == row && pg.second.col == col)
                    {
                        if (Creature* occPiece = botAI->GetCreature(pg.first))
                            occBy = occPiece->GetName() + "/" + pg.first.ToString();
                        else
                            occBy = pg.first.ToString();
                        break;
                    }
                }
            }

            if (!candDbg.empty())
                candDbg += ";";
            candDbg += "cand_sq=(" + std::to_string(row) + "," + std::to_string(col) + ")" +
                       " world=(" + std::to_string(candWorld.GetPositionX()) + "," + std::to_string(candWorld.GetPositionY()) + "," + std::to_string(candWorld.GetPositionZ()) + ")" +
                       " in_bounds=" + std::string(inBounds ? "1" : "0") +
                       " occupied=" + std::string(occupied ? "1" : "0") +
                       " occupied_by=" + occBy;

            if (!inBounds)
                continue;
            if (occupied)
                continue;

            ObjectGuid reservedBy;
            if (IsChessDestinationReserved(instanceId, ChessSquare{ row, col }, nowTick,
                pieceGuid, reservedBy))
                continue;

            float candidateScore = 0.0f;
            ChessPositionEvaluation candidateEvaluation;
            if (pawnOpening)
            {
                if (row != curSq.row + pawnOpeningForward)
                    continue;

                // Stagger four pawn lanes diagonally. This opens both rooks and bishops without
                // building another straight pawn wall directly in front of the back-rank pieces.
                candidateScore += col == pawnOpeningPreferredCol ? 10000.0f :
                    (col == curSq.col ? 1000.0f : 0.0f);
            }
            else if (moveTarget)
            {
                candidateEvaluation = EvaluateChessPosition(bot, piece, board, mv.second, moveTarget,
                    tacticalHealing, ChessSquare{ row, col }, curSq);
                candidateScore = candidateEvaluation.score;
            }

            if (!bestForwardTrigger || candidateScore > bestMoveScore ||
                (candidateScore == bestMoveScore && std::pair<int, int>{ row, col } <
                    std::pair<int, int>{ bestRow, bestCol }))
            {
                bestRow = row;
                bestCol = col;
                bestMoveScore = candidateScore;
                bestForwardTrigger = mv.second;
                bestEvaluation = candidateEvaluation;
            }
        }

        if (!bestForwardTrigger)
        {
            if (moveTarget)
                RegisterChessTargetFailure(bot, piece, moveTarget->GetGUID(), now, "no_scored_destination");
            LogChessDecision(bot, piece, "move-failed", "reason=no_legal_candidate target=" +
                (moveTarget ? moveTarget->GetName() : "none") + " current_square=" + ChessSquareToString(curSq));
            return false;
        }

        if (!pawnOpening && currentEvaluation.intendedTargetAttackable &&
            bestMoveScore <= currentEvaluation.score + 250.0f)
        {
            LogChessDecision(bot, piece, "hold-position", "target=" + moveTarget->GetName() +
                " reason=no_materially_better_square current_score=" + std::to_string(currentEvaluation.score) +
                " best_candidate_score=" + std::to_string(bestMoveScore));
            return false;
        }

        int toRow = -1, toCol = -1;
        WorldToChessSquare(board, bestForwardTrigger->GetPositionX(), bestForwardTrigger->GetPositionY(), toRow, toCol);
        bool occupied = board.occupied.find({ toRow, toCol }) != board.occupied.end();
        if (occupied)
        {
            std::string occBy = "unknown";
            for (auto const& pg : board.pieceSquare)
            {
                if (pg.second.row == toRow && pg.second.col == toCol)
                {
                    if (Creature* occPiece = botAI->GetCreature(pg.first))
                        occBy = occPiece->GetName() + "/" + pg.first.ToString();
                    else
                        occBy = pg.first.ToString();
                    break;
                }
            }
            return false;
        }

        if (!moveSpell || piece->HasSpellCooldown(moveSpell))
        {
            return false;
        }

        SpellCastResult const moveResult = piece->CastSpell(bestForwardTrigger, moveSpell, true);
        if (!CompleteChessSpellCast(piece, moveSpell, moveResult))
        {
            chessLastFailedMoveTriggerByPiece[pieceGuid] = bestForwardTrigger->GetGUID();
            chessLastFailedMoveTimeByPiece[pieceGuid] = nowTick;
            BackoffChessMoveTrigger(pieceGuid, bestForwardTrigger->GetGUID(), nowTick);
            chessMovementCooldownUntilByPiece[pieceGuid] = nowTick + 1;
            if (moveTarget)
                RegisterChessTargetFailure(bot, piece, moveTarget->GetGUID(), nowTick, "move_cast_rejected");
            LogChessDecision(bot, piece, "move-cast-failed", "spell=" + std::to_string(moveSpell) + " from=" +
                ChessSquareToString(curSq) + " to=" + ChessSquareToString(ChessSquare{ toRow, toCol }) +
                " target=" + (moveTarget ? moveTarget->GetName() : "none") + " cast_result=" +
                std::to_string(static_cast<uint32>(moveResult)));
            return false;
        }

        RecordPendingChessMove(
            bot, piece, instanceId, curSq, ChessSquare{ toRow, toCol }, moveSpell,
            bestForwardTrigger->GetGUID(), pawnOpening ? "opening-pawn" : "tactical", nowTick, 8,
            moveTarget ? moveTarget->GetGUID() : ObjectGuid::Empty);
        LogChessDecision(bot, piece, "move-command", "target=" +
            (moveTarget ? moveTarget->GetName() : "enemy-king-opening") + " from=" + ChessSquareToString(curSq) +
            " to=" + ChessSquareToString(ChessSquare{ toRow, toCol }) + " reason=" + moveTargetReason +
            " score=" + std::to_string(bestMoveScore) + " target_distance=" +
            std::to_string(bestEvaluation.distance) + " preferred_distance=" +
            std::to_string(bestEvaluation.preferredDistance) + " abilities_in_range=" +
            std::to_string(bestEvaluation.abilitiesInRange) + " ready_abilities_in_range=" +
            std::to_string(bestEvaluation.readyAbilitiesInRange) + " additional_targets=" +
            std::to_string(bestEvaluation.additionalTargets) + " nearby_enemies=" +
            std::to_string(bestEvaluation.nearbyEnemies), true);
        return true;
    }

    return false;
}

bool KarazhanChessMoveOutOfFireAction::Execute(Event /*event*/)
{
    Creature* piece = bot->GetCharm() ? bot->GetCharm()->ToCreature() : nullptr;
    if (!piece || !IsChessPieceEntry(piece->GetEntry()))
        return false;
    if (!IsChessEncounterRelevant(botAI, bot))
        return false;
    const uint32 instanceId = bot->GetMap() ? bot->GetMap()->GetInstanceId() : 0;
    bool const openingProgressConfirmed = HasConfirmedOpeningProgress(instanceId);
    bool const controlledNonPawn = !IsPawnEntry(piece->GetEntry()) && openingProgressConfirmed;
    if (!IsPawnEntry(piece->GetEntry()) && !controlledNonPawn)
        return false;
    if (!IsChessPieceCommandGcdReady(piece))
    {
        return false;
    }
    const time_t now = std::time(nullptr);
    if (controlledNonPawn)
    {
    }

    std::vector<Creature*> hazards;
    GuidVector const npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();
    for (ObjectGuid const& npcGuid : npcs)
    {
        Creature* creature = botAI->GetCreature(npcGuid);
        if (creature && creature->GetEntry() == NPC_CHESS_EVENT_MEDIVH_CHEAT_FIRES)
            hazards.push_back(creature);
    }

    ChessBoardState board = BuildChessBoardState(botAI, bot);
    ChessSquare controlledSq;
    std::string controlledRejectReason;
    if (!IsActiveBoardFriendlyControlledChessPiece(bot, board, piece, controlledSq, controlledRejectReason))
    {
        HandleInvalidControlledChessPiece(botAI, bot, piece, board, "move_out_of_fire", controlledRejectReason);
        return false;
    }
    ChessPhase phase = GetChessPhase(botAI, bot, board);
    std::string confirmSource;
    ChessPendingMoveResult const pendingResult = TryResolvePendingChessMove(botAI, bot, piece, board, now, confirmSource);
    if (pendingResult == ChessPendingMoveResult::CONFIRMED || pendingResult == ChessPendingMoveResult::WAITING || pendingResult == ChessPendingMoveResult::EXPIRED)
        return true;

    if (!IsChessMoveReady(piece, now, 5))
    {
        return false;
    }

    if (phase == ChessPhase::OPENING && HasPawnMovedDuringOpening(instanceId, piece->GetGUID()))
    {
        return false;
    }
    auto retryIt = chessOpeningMoveRetryUntilByPiece.find(piece->GetGUID());
    if (phase == ChessPhase::OPENING && retryIt != chessOpeningMoveRetryUntilByPiece.end() && now < retryIt->second)
    {
        return false;
    }
    ChessSquare oldSq;
    bool const hasOldSq = board.pieceSquare.find(piece->GetGUID()) != board.pieceSquare.end();
    if (hasOldSq)
        oldSq = board.pieceSquare.find(piece->GetGUID())->second;
    std::string legalDbg;
    std::vector<Creature*> triggers = GetLegalMoveTriggersForPiece(botAI, bot, piece, board, legalDbg);
    uint32 const moveSpell = GetChessMoveSpellForPiece(piece);
    Creature* safe = nullptr;
    for (Creature* trigger : triggers)
    {
        int candidateRow = -1;
        int candidateCol = -1;
        ObjectGuid reservedBy;
        if (!WorldToChessSquare(board, trigger->GetPositionX(), trigger->GetPositionY(), candidateRow, candidateCol) ||
            IsChessDestinationReserved(instanceId, ChessSquare{ candidateRow, candidateCol }, now,
                piece->GetGUID(), reservedBy))
            continue;
        if (IsChessMoveTriggerBackedOff(piece->GetGUID(), trigger->GetGUID(), now))
            continue;

        bool bad = false;
        for (Creature* hz : hazards)
        {
            if (trigger->GetExactDist2d(hz) < 3.5f)
            {
                bad = true;
                break;
            }
        }
        if (!bad)
        {
            safe = trigger;
            break;
        }
    }

    if (!safe)
    {
        return false;
    }

    int toRow = -1;
    int toCol = -1;
    bool const destMapped = WorldToChessSquare(board, safe->GetPositionX(), safe->GetPositionY(), toRow, toCol);
    if (!destMapped || !IsInsideBoard(board, toRow, toCol))
    {
        return false;
    }

    if (!moveSpell || piece->HasSpellCooldown(moveSpell))
    {
        return false;
    }

    SpellCastResult const moveResult = piece->CastSpell(safe, moveSpell, true);
    if (!CompleteChessSpellCast(piece, moveSpell, moveResult))
    {
        chessLastFailedMoveTriggerByPiece[piece->GetGUID()] = safe->GetGUID();
        chessLastFailedMoveTimeByPiece[piece->GetGUID()] = now;
        BackoffChessMoveTrigger(piece->GetGUID(), safe->GetGUID(), now);
        LogChessDecision(bot, piece, "move-cast-failed", "spell=" + std::to_string(moveSpell) +
            " source=move-out-of-fire destination=" + ChessSquareToString(ChessSquare{ toRow, toCol }) +
            " cast_result=" + std::to_string(static_cast<uint32>(moveResult)));
        return false;
    }

    RecordPendingChessMove(
        bot, piece, instanceId, hasOldSq ? oldSq : ChessSquare{}, ChessSquare{ toRow, toCol }, moveSpell,
        safe->GetGUID(), "move-out-of-fire", now, 8);
    return true;
}

bool KarazhanChessUseAbilityAction::Execute(Event /*event*/)
{
    Creature* piece = bot->GetCharm() ? bot->GetCharm()->ToCreature() : nullptr;
    if (!piece || !IsChessPieceEntry(piece->GetEntry()))
        return false;
    if (!IsChessEncounterRelevant(botAI, bot))
        return false;
    const uint32 instanceId = bot->GetMap() ? bot->GetMap()->GetInstanceId() : 0;
    bool const openingProgressConfirmed = HasConfirmedOpeningProgress(instanceId);
    bool const controlledNonPawn = !IsPawnEntry(piece->GetEntry()) && openingProgressConfirmed;
    if (!IsPawnEntry(piece->GetEntry()) && !controlledNonPawn)
    {
        return false;
    }
    if (controlledNonPawn)
    {
    }
    if (!IsChessPieceCommandGcdReady(piece))
    {
        return false;
    }

    Creature* enemyKing = GetEnemyChessKing(botAI, bot);
    const bool canMove = !piece->HasAura(SPELL_MOVE_COOLDOWN);
    const bool needsMovement = canMove && enemyKing && piece->GetExactDist2d(enemyKing) > 22.0f;
    const time_t now = std::time(nullptr);
    const ObjectGuid pieceGuid = piece->GetGUID();
    if (HasPendingChessMove(pieceGuid, now))
        return false;
    const bool isPawn = piece->GetEntry() == NPC_PAWN_A || piece->GetEntry() == NPC_PAWN_H;
    const float kingDist = enemyKing ? piece->GetExactDist2d(enemyKing) : 999.0f;
    ChessBoardState board = BuildChessBoardState(botAI, bot);
    ChessSquare controlledSq;
    std::string controlledRejectReason;
    if (!IsActiveBoardFriendlyControlledChessPiece(bot, board, piece, controlledSq, controlledRejectReason))
    {
        HandleInvalidControlledChessPiece(botAI, bot, piece, board, "use_ability", controlledRejectReason);
        return false;
    }
    ChessSupportTargetSelection healingTarget;
    if (IsHealerChessPieceEntry(piece->GetEntry()))
        healingTarget = SelectDamagedFriendlyActiveBoardTarget(bot, board, piece);
    bool const healerNeedsTarget = healingTarget.target != nullptr && GetChessHealSpell(piece, true) != 0;
    ChessPhase phase = GetChessPhase(botAI, bot, board);
    bool const openingPawnMoved = openingProgressConfirmed ||
        (instanceId && chessOpenedLanePawnsByInstance.count(instanceId) &&
        !chessOpenedLanePawnsByInstance[instanceId].empty());
    bool const nonpawnPriorityBypass = controlledNonPawn;
    uint32 const abilityWindowSec = GetChessAbilityWindowSec(controlledNonPawn);
    uint32 enemySupportAlive = 0;
    uint32 enemyDamageAlive = 0;
    uint32 enemyPawnAlive = 0;
    std::string kingGateReason = "no_enemy_king";
    bool const kingAttackAllowed = enemyKing && IsKarazhanChessKingFocusAllowedActiveBoard(bot, board, enemyKing, enemySupportAlive, enemyDamageAlive, enemyPawnAlive, kingGateReason);
    const bool hasValidChessTarget = healerNeedsTarget || enemyKing;

    if (enemyKing)
    {
    }

    if (nonpawnPriorityBypass)
    {
    }

    bool const useNonKingOffensePrecheck = enemyKing && !kingAttackAllowed;
    ChessOffensiveTargetSelection nonKingTarget;
    if (useNonKingOffensePrecheck)
    {
        nonKingTarget = SelectPersistentChessTarget(botAI, bot, piece, board, "offense");

        if (!nonKingTarget.target)
        {
        }
        else
        {
            if (IsSummonedDaemonChessPiece(piece->GetEntry()))
            {
                if (nonKingTarget.category == "pawn")
                {
                }
            }
        }
    }

    if (!IsChessAbilityReady(piece, now, abilityWindowSec))
    {
        time_t const lastAbility = chessLastAbilityCommandByPiece.count(pieceGuid) ? chessLastAbilityCommandByPiece[pieceGuid] : 0;
        return false;
    }

    std::vector<uint32> spellCandidates;
    ChessSupportTargetSelection supportTarget;
    if (controlledNonPawn && !healerNeedsTarget)
    {
        spellCandidates = GetLikelyOffensiveChessSpells(piece);
        if (piece->GetEntry() == NPC_BISHOP_H || piece->GetEntry() == NPC_BISHOP_A)
        {
            for (uint8 i = 0; i < MAX_CREATURE_SPELLS; ++i)
            {
                uint32 const candidateSpellId = piece->m_spells[i];
                if (!candidateSpellId)
                    continue;

                std::string offensiveRejectReason;
                bool const offensiveAllowed = IsKingAttackOffensiveChessSpell(candidateSpellId, offensiveRejectReason);
                std::string supportRejectReason;
                bool const supportAllowed = IsChessSupportSpell(candidateSpellId, supportRejectReason);
            }
        }
        if (spellCandidates.empty())
        {
            supportTarget = SelectDamagedFriendlyActiveBoardTarget(bot, board, piece);

            if (!supportTarget.target)
            {
                return false;
            }

            for (uint8 i = 0; i < MAX_CREATURE_SPELLS; ++i)
            {
                uint32 const supportSpellId = piece->m_spells[i];
                if (!supportSpellId)
                    continue;

                std::string supportRejectReason;
                bool const supportAllowed = IsChessSupportSpell(supportSpellId, supportRejectReason);
                if (supportAllowed)
                    spellCandidates.push_back(supportSpellId);
            }

            if (spellCandidates.empty())
            {
                return false;
            }

        }
    }
    else
    {
        for (uint8 i = 0; i < MAX_CREATURE_SPELLS; ++i)
        {
            uint32 const spellId = piece->m_spells[i];
            if (spellId)
                spellCandidates.push_back(spellId);
        }
    }

    bool const noActionableNonKingTarget =
        useNonKingOffensePrecheck &&
        !nonKingTarget.target &&
        IsNoActionableNonKingRejectReason(nonKingTarget.rejectReason);
    bool const noActionableMovementTarget =
        noActionableNonKingTarget &&
        canMove &&
        kingDist > 20.0f;
    bool const offensiveSpellsAvailable = !spellCandidates.empty();
    bool const allowKingFocusDueToNoActionableNonKing =
        enemyKing &&
        !kingAttackAllowed &&
        kingGateReason == "board_not_cleared" &&
        noActionableNonKingTarget &&
        offensiveSpellsAvailable &&
        (enemyDamageAlive <= 1 || noActionableMovementTarget);
    bool const effectiveKingAttackAllowed = kingAttackAllowed || allowKingFocusDueToNoActionableNonKing;
    bool const useNonKingOffense = enemyKing && !effectiveKingAttackAllowed;
    std::string kingFallbackRejectedReason = "none";
    if (!enemyKing)
        kingFallbackRejectedReason = "no_enemy_king";
    else if (kingAttackAllowed)
        kingFallbackRejectedReason = "king_gate_allowed";
    else if (kingGateReason != "board_not_cleared")
        kingFallbackRejectedReason = "gate_" + kingGateReason;
    else if (!noActionableNonKingTarget)
        kingFallbackRejectedReason = "nonking_target_actionable";
    else if (!offensiveSpellsAvailable)
        kingFallbackRejectedReason = "no_offensive_spells";
    else if (enemyDamageAlive > 1)
        kingFallbackRejectedReason = "enemy_damage_alive_gt_1";
    else if (!noActionableMovementTarget)
        kingFallbackRejectedReason = "movement_path_available_or_within_range";

    if (allowKingFocusDueToNoActionableNonKing)
    {
    }
    else
    {
    }

    // Opening behavior: pawns should clear lanes before spamming abilities.
    if (isPawn && canMove && kingDist > 14.0f && !openingPawnMoved)
    {
        return false;
    }

    // Keep visual spam down and prioritize board progress when still far from enemy king.
    if (!isPawn && canMove && kingDist > 20.0f && !healerNeedsTarget && !nonpawnPriorityBypass)
    {
        return false;
    }

    for (uint32 spellId : spellCandidates)
    {
        if (!spellId || piece->HasSpellCooldown(spellId))
            continue;

            time_t attackBackoffRemaining = 0;
            if (IsChessSpellNoOpBackoffActive(pieceGuid, spellId, now, attackBackoffRemaining))
            {
                continue;
            }

            ChessSquare targetSquare;
            bool targetBoardSquareFound = false;
            bool targetActiveBoardPiece = false;
            if (healerNeedsTarget)
            {
                std::string supportRejectReason;
                if (!IsChessSupportSpell(spellId, supportRejectReason))
                    continue;

                if (piece->GetEntry() == NPC_BISHOP_H || piece->GetEntry() == NPC_BISHOP_A)
                {
                }
                if (!IsChessSpellTargetInRange(piece, healingTarget.target, spellId))
                    continue;

                uint32 const healthBefore = healingTarget.target->GetHealth();
                if (CastChessSpell(piece, healingTarget.target, spellId))
                {
                    uint32 const healthAfter = healingTarget.target->GetHealth();
                    ClearChessSpellNoOpBackoff(pieceGuid, spellId);
                    StampChessPieceCommandGcd(piece);
                    chessLastAbilityCommandByPiece[pieceGuid] = now;
                    LogChessDecision(bot, piece, "heal-cast", "spell=" + std::to_string(spellId) + "/" +
                        GetChessSpellName(spellId) + " target=" + healingTarget.target->GetName() + " distance=" +
                        std::to_string(piece->GetExactDist2d(healingTarget.target)) + " health_before=" +
                        std::to_string(healthBefore) + " health_after=" + std::to_string(healthAfter) +
                        " restored=" + std::to_string(static_cast<int64>(healthAfter) - healthBefore) +
                        " restoration_verified=" + (healthAfter > healthBefore ? "1" : "0"), true);
                    return true;
                }
                LogChessDecision(bot, piece, "heal-cast-failed", "spell=" + std::to_string(spellId) +
                    " target=" + healingTarget.target->GetName() + " distance=" +
                    std::to_string(piece->GetExactDist2d(healingTarget.target)), true);
                StampChessSpellNoOpBackoff(pieceGuid, spellId, now, 2);
                continue;
            }

            if (supportTarget.target)
            {
                auto supportSqIt = board.pieceSquare.find(supportTarget.target->GetGUID());
                bool const supportTargetValid =
                    supportTarget.target->IsInWorld() &&
                    supportTarget.target->IsAlive() &&
                    IsChessPieceEntry(supportTarget.target->GetEntry()) &&
                    IsFriendlyChessPieceForBot(bot, supportTarget.target) &&
                    supportSqIt != board.pieceSquare.end() &&
                    board.occupied.find({ supportSqIt->second.row, supportSqIt->second.col }) != board.occupied.end();
                if (!supportTargetValid)
                {
                    continue;
                }

                if (!IsChessSpellTargetInRange(piece, supportTarget.target, spellId))
                    continue;

                if (CastChessSpell(piece, supportTarget.target, spellId))
                {
                    ClearChessSpellNoOpBackoff(pieceGuid, spellId);
                    StampChessPieceCommandGcd(piece);
                    chessLastAbilityCommandByPiece[pieceGuid] = now;
                    return true;
                }

                StampChessSpellNoOpBackoff(pieceGuid, spellId, now, 2);
                continue;
            }

            std::string offensiveRejectReason;
            bool const offensiveKingSpell = IsKingAttackOffensiveChessSpell(spellId, offensiveRejectReason);
            if (piece->GetEntry() == NPC_BISHOP_H || piece->GetEntry() == NPC_BISHOP_A)
            {
            }
            if (enemyKing && offensiveKingSpell && effectiveKingAttackAllowed)
        {
            if (phase == ChessPhase::OPENING && !openingPawnMoved)
            {
                continue;
            }
            std::string validationReason;
            bool alive = false, inWorld = false, chessPiece = false, enemySide = false, isKing = false;
            if (!ValidateLiveChessBoardTargetForCast(bot, board, enemyKing, true, validationReason, alive, inWorld, chessPiece, enemySide, isKing, targetSquare, targetBoardSquareFound, targetActiveBoardPiece))
            {
                continue;
            }
            if (IsPoisonCloudChessSpell(spellId))
            {
                std::string poisonReason;
                if (ShouldThrottlePoisonCloudCast(bot, piece, enemyKing, spellId, now, poisonReason))
                    continue;
            }
            if (!IsChessSpellTargetInRange(piece, enemyKing, spellId))
                continue;
            if (IsChessHealSpellBlockedOnEnemy(spellId))
                continue;
            if (CastOffensiveChessSpell(piece, enemyKing, spellId))
            {
                ClearChessSpellNoOpBackoff(pieceGuid, spellId);
                StampChessPieceCommandGcd(piece);
                chessLastAbilityCommandByPiece[pieceGuid] = now;
                if (IsPoisonCloudChessSpell(spellId))
                    chessPoisonCloudLastAppliedByTarget[enemyKing->GetGUID()] = now;
                LogChessDecision(bot, piece, "attack-cast", "spell=" + std::to_string(spellId) + "/" +
                    GetChessSpellName(spellId) + " shape=" + GetChessSpellShapeLabel(spellId) + " target=" +
                    enemyKing->GetName() + " distance=" + std::to_string(piece->GetExactDist2d(enemyKing)) +
                    " effective_range=" + std::to_string(GetChessOffensiveSpellRange(piece, spellId)) +
                    " target_kind=king", true);
                return true;
            }
            LogChessDecision(bot, piece, "attack-cast-failed", "spell=" + std::to_string(spellId) +
                " target=" + enemyKing->GetName() + " distance=" +
                std::to_string(piece->GetExactDist2d(enemyKing)), true);
            StampChessSpellNoOpBackoff(pieceGuid, spellId, now, 2);
            continue;
        }
        if (useNonKingOffense && nonKingTarget.target && offensiveKingSpell && !effectiveKingAttackAllowed)
        {
            if (phase == ChessPhase::OPENING && !openingPawnMoved)
            {
                continue;
            }

            if (IsCasterCenteredOffensiveChessSpell(spellId))
            {
                float const aoeRadius = GetChessSpellEffectRadius(piece, spellId);
                if (piece->GetExactDist2d(nonKingTarget.target) > aoeRadius)
                {
                    LogChessDecision(bot, piece, "attack-wait", "spell=" + std::to_string(spellId) + "/" +
                        GetChessSpellName(spellId) + " target=" + nonKingTarget.target->GetName() +
                        " reason=intended_target_outside_area distance=" +
                        std::to_string(piece->GetExactDist2d(nonKingTarget.target)) + " effective_range=" +
                        std::to_string(aoeRadius));
                    continue;
                }
            }

            if (IsPoisonCloudChessSpell(spellId))
            {
                std::string poisonReason;
                if (ShouldThrottlePoisonCloudCast(bot, piece, nonKingTarget.target, spellId, now, poisonReason))
                    continue;
            }
            if (IsChessHealSpellBlockedOnEnemy(spellId))
                continue;

            if (!IsChessSpellTargetInRange(piece, nonKingTarget.target, spellId))
                continue;

            time_t nonKingBackoffRemaining = 0;
            if (IsChessSpellNoOpBackoffActive(pieceGuid, spellId, now, nonKingBackoffRemaining))
            {
                continue;
            }

            std::string validationReason;
            bool alive = false, inWorld = false, chessPiece = false, enemySide = false, isKing = false;
            if (!ValidateLiveChessBoardTargetForCast(bot, board, nonKingTarget.target, false, validationReason, alive, inWorld, chessPiece, enemySide, isKing, targetSquare, targetBoardSquareFound, targetActiveBoardPiece))
            {
                continue;
            }
            if (CastOffensiveChessSpell(piece, nonKingTarget.target, spellId))
            {
                ClearChessSpellNoOpBackoff(pieceGuid, spellId);
                StampChessPieceCommandGcd(piece);
                chessLastAbilityCommandByPiece[pieceGuid] = now;
                if (IsPoisonCloudChessSpell(spellId))
                    chessPoisonCloudLastAppliedByTarget[nonKingTarget.target->GetGUID()] = now;
                LogChessDecision(bot, piece, "attack-cast", "spell=" + std::to_string(spellId) + "/" +
                    GetChessSpellName(spellId) + " shape=" + GetChessSpellShapeLabel(spellId) + " target=" +
                    nonKingTarget.target->GetName() + " distance=" +
                    std::to_string(piece->GetExactDist2d(nonKingTarget.target)) + " effective_range=" +
                    std::to_string(GetChessOffensiveSpellRange(piece, spellId)) + " target_kind=" +
                    nonKingTarget.category, true);
                return true;
            }

            LogChessDecision(bot, piece, "attack-cast-failed", "spell=" + std::to_string(spellId) +
                " target=" + nonKingTarget.target->GetName() + " distance=" +
                std::to_string(piece->GetExactDist2d(nonKingTarget.target)), true);
            StampChessSpellNoOpBackoff(pieceGuid, spellId, now, 2);
            continue;
        }
        if (enemyKing && offensiveKingSpell && !effectiveKingAttackAllowed)
        {
            if (useNonKingOffense)
            {
            }

            if (IsCasterCenteredOffensiveChessSpell(spellId))
            {
                uint32 nearbyEnemyCount = CountNearbyActiveBoardEnemyChessPieces(
                    bot, board, piece, GetChessSpellEffectRadius(piece, spellId));
                if (!nearbyEnemyCount)
                {
                    continue;
                }
            }

            ChessSquare targetSquare;
            bool targetBoardSquareFound = false;
            bool targetActiveBoardPiece = false;
            std::string validationReason;
            bool alive = false, inWorld = false, chessPiece = false, enemySide = false, isKing = false;
            if (!ValidateLiveChessBoardTargetForCast(bot, board, enemyKing, true, validationReason, alive, inWorld, chessPiece, enemySide, isKing, targetSquare, targetBoardSquareFound, targetActiveBoardPiece))
            {
                continue;
            }
            continue;
        }
        if (useNonKingOffense && !nonKingTarget.target)
        {
        }
        if (needsMovement || hasValidChessTarget)
        {
            continue;
        }

        time_t selfBackoffRemaining = 0;
        if (IsChessSpellNoOpBackoffActive(pieceGuid, spellId, now, selfBackoffRemaining))
        {
            continue;
        }

        time_t& selfThrottle = chessSelfAbilityThrottleByPiece[piece->GetGUID()];
        if ((now - selfThrottle) < 8)
        {
            continue;
        }

        if (CastChessSpell(piece, piece, spellId))
        {
            selfThrottle = now;
            StampChessPieceCommandGcd(piece);
            chessLastAbilityCommandByPiece[pieceGuid] = now;
            ClearChessSpellNoOpBackoff(pieceGuid, spellId);
            return true;
        }
        StampChessSpellNoOpBackoff(pieceGuid, spellId, now, 2);
    }

    bool const utilityUseful = piece->GetHealthPct() < 85.0f ||
        CountNearbyEnemyChessPieces(botAI, bot, piece, 10.0f) > 0;
    if (utilityUseful)
    {
        for (uint8 i = 0; i < MAX_CREATURE_SPELLS; ++i)
        {
            uint32 const spellId = piece->m_spells[i];
            if (!IsChessSelfUtilitySpell(spellId) || piece->HasSpellCooldown(spellId))
                continue;

            if (CastChessSpell(piece, piece, spellId))
            {
                StampChessPieceCommandGcd(piece);
                chessLastAbilityCommandByPiece[pieceGuid] = now;
                LogChessDecision(bot, piece, "utility-cast", "spell=" + std::to_string(spellId) + "/" +
                    GetChessSpellName(spellId) + " health_pct=" + std::to_string(piece->GetHealthPct()), true);
                return true;
            }
        }
    }

    return false;
}

bool KarazhanChessHealFriendlyAction::Execute(Event event)
{
    (void)event;
    Creature* piece = bot->GetCharm() ? bot->GetCharm()->ToCreature() : nullptr;
    if (!piece || !IsHealerChessPieceEntry(piece->GetEntry()))
        return false;
    if (!IsChessEncounterRelevant(botAI, bot))
        return false;
    const uint32 instanceId = bot->GetMap() ? bot->GetMap()->GetInstanceId() : 0;
    bool const openingProgressConfirmed = HasConfirmedOpeningProgress(instanceId);
    bool const controlledNonPawn = !IsPawnEntry(piece->GetEntry()) && openingProgressConfirmed;
    if (!IsPawnEntry(piece->GetEntry()) && !controlledNonPawn)
    {
        return false;
    }
    if (controlledNonPawn)
    {
    }
    if (!IsChessPieceCommandGcdReady(piece))
        return false;
    ChessBoardState board = BuildChessBoardState(botAI, bot);
    ChessSquare controlledSq;
    std::string controlledRejectReason;
    if (!IsActiveBoardFriendlyControlledChessPiece(bot, board, piece, controlledSq, controlledRejectReason))
    {
        HandleInvalidControlledChessPiece(botAI, bot, piece, board, "heal_friendly", controlledRejectReason);
        return false;
    }
    ChessSupportTargetSelection healingTarget = SelectDamagedFriendlyActiveBoardTarget(bot, board, piece);
    if (!healingTarget.target)
        return false;
    const time_t now = std::time(nullptr);
    if (HasPendingChessMove(piece->GetGUID(), now))
        return false;
    uint32 const abilityWindowSec = GetChessAbilityWindowSec(controlledNonPawn);

    if (!IsChessAbilityReady(piece, now, abilityWindowSec))
    {
        time_t const lastAbility = chessLastAbilityCommandByPiece.count(piece->GetGUID()) ? chessLastAbilityCommandByPiece[piece->GetGUID()] : 0;
        return false;
    }

    for (uint8 i = 0; i < MAX_CREATURE_SPELLS; ++i)
    {
        uint32 spellId = piece->m_spells[i];
        if (spellId && !piece->HasSpellCooldown(spellId))
        {
            std::string rejectReason;
            bool const supportAllowed = IsChessSupportSpell(spellId, rejectReason);
            if (!supportAllowed)
            {
                continue;
            }

            time_t backoffRemaining = 0;
            if (IsChessSpellNoOpBackoffActive(piece->GetGUID(), spellId, now, backoffRemaining))
            {
                continue;
            }

            if (!IsChessSpellTargetInRange(piece, healingTarget.target, spellId))
                continue;

            uint32 const healthBefore = healingTarget.target->GetHealth();
            if (CastChessSpell(piece, healingTarget.target, spellId))
            {
                uint32 const healthAfter = healingTarget.target->GetHealth();
                ClearChessSpellNoOpBackoff(piece->GetGUID(), spellId);
                StampChessPieceCommandGcd(piece);
                chessLastAbilityCommandByPiece[piece->GetGUID()] = now;
                LogChessDecision(bot, piece, "heal-cast", "spell=" + std::to_string(spellId) + "/" +
                    GetChessSpellName(spellId) + " target=" + healingTarget.target->GetName() + " distance=" +
                    std::to_string(piece->GetExactDist2d(healingTarget.target)) + " health_before=" +
                    std::to_string(healthBefore) + " health_after=" + std::to_string(healthAfter) +
                    " restored=" + std::to_string(static_cast<int64>(healthAfter) - healthBefore) +
                    " restoration_verified=" + (healthAfter > healthBefore ? "1" : "0"), true);
                return true;
            }

            LogChessDecision(bot, piece, "heal-cast-failed", "spell=" + std::to_string(spellId) +
                " target=" + healingTarget.target->GetName() + " distance=" +
                std::to_string(piece->GetExactDist2d(healingTarget.target)), true);
            StampChessSpellNoOpBackoff(piece->GetGUID(), spellId, now, 2);
        }
    }

    return false;
}

bool KarazhanChessAttackEnemyKingAction::Execute(Event /*event*/)
{
    Creature* piece = bot->GetCharm() ? bot->GetCharm()->ToCreature() : nullptr;
    if (!piece)
        return false;
    if (!IsChessEncounterRelevant(botAI, bot))
        return false;
    Creature* enemyKing = GetEnemyChessKing(botAI, bot);
    if (!enemyKing)
        return false;
    const uint32 instanceId = bot->GetMap() ? bot->GetMap()->GetInstanceId() : 0;
    bool const openingProgressConfirmed = HasConfirmedOpeningProgress(instanceId);
    bool const controlledNonPawn = !IsPawnEntry(piece->GetEntry()) && openingProgressConfirmed;
    if (!IsPawnEntry(piece->GetEntry()) && !controlledNonPawn)
    {
        return false;
    }
    if (controlledNonPawn)
    {
    }
    if (!IsChessPieceCommandGcdReady(piece))
        return false;
    ChessBoardState board = BuildChessBoardState(botAI, bot);
    ChessSquare controlledSq;
    std::string controlledRejectReason;
    if (!IsActiveBoardFriendlyControlledChessPiece(bot, board, piece, controlledSq, controlledRejectReason))
    {
        HandleInvalidControlledChessPiece(botAI, bot, piece, board, "attack_enemy_king", controlledRejectReason);
        return false;
    }
    const time_t now = std::time(nullptr);
    if (HasPendingChessMove(piece->GetGUID(), now))
        return false;
    uint32 const abilityWindowSec = GetChessAbilityWindowSec(controlledNonPawn);

    if (!IsChessAbilityReady(piece, now, abilityWindowSec))
    {
        time_t const lastAbility = chessLastAbilityCommandByPiece.count(piece->GetGUID()) ? chessLastAbilityCommandByPiece[piece->GetGUID()] : 0;
        return false;
    }
    ChessPhase phase = GetChessPhase(botAI, bot, board);
    uint32 enemySupportAlive = 0;
    uint32 enemyDamageAlive = 0;
    uint32 enemyPawnAlive = 0;
    std::string kingGateReason = "no_enemy_king";
    bool const kingAttackAllowed = IsKarazhanChessKingFocusAllowedActiveBoard(bot, board, enemyKing, enemySupportAlive, enemyDamageAlive, enemyPawnAlive, kingGateReason);
    ChessOffensiveTargetSelection nonKingTarget =
        SelectPersistentChessTarget(botAI, bot, piece, board, "offense");
    bool const noActionableNonKingTarget =
        !nonKingTarget.target &&
        IsNoActionableNonKingRejectReason(nonKingTarget.rejectReason);
    bool const offensiveSpellsAvailable = !GetLikelyOffensiveChessSpells(piece).empty();
    bool const allowKingFocusDueToNoActionableNonKing =
        !kingAttackAllowed &&
        kingGateReason == "board_not_cleared" &&
        noActionableNonKingTarget &&
        offensiveSpellsAvailable &&
        enemyDamageAlive <= 1;
    bool const effectiveKingAttackAllowed = kingAttackAllowed || allowKingFocusDueToNoActionableNonKing;
    std::string kingFallbackRejectedReason = "none";
    if (!enemyKing)
        kingFallbackRejectedReason = "no_enemy_king";
    else if (kingAttackAllowed)
        kingFallbackRejectedReason = "king_gate_allowed";
    else if (kingGateReason != "board_not_cleared")
        kingFallbackRejectedReason = "gate_" + kingGateReason;
    else if (!noActionableNonKingTarget)
        kingFallbackRejectedReason = "nonking_target_actionable";
    else if (!offensiveSpellsAvailable)
        kingFallbackRejectedReason = "no_offensive_spells";
    else if (enemyDamageAlive > 1)
        kingFallbackRejectedReason = "enemy_damage_alive_gt_1";

    if (allowKingFocusDueToNoActionableNonKing)
    {
    }
    else
    {
    }

    if (!effectiveKingAttackAllowed)
        return false;

    bool const openingPawnMoved = openingProgressConfirmed ||
        (instanceId && chessOpenedLanePawnsByInstance.count(instanceId) &&
        !chessOpenedLanePawnsByInstance[instanceId].empty());
    if (phase == ChessPhase::OPENING && !openingPawnMoved)
    {
        return false;
    }

    for (uint8 i = 0; i < MAX_CREATURE_SPELLS; ++i)
    {
        uint32 spellId = piece->m_spells[i];
        if (spellId && !piece->HasSpellCooldown(spellId))
        {
            std::string offensiveRejectReason;
            if (!IsKingAttackOffensiveChessSpell(spellId, offensiveRejectReason))
                continue;

            time_t backoffRemaining = 0;
            if (IsChessSpellNoOpBackoffActive(piece->GetGUID(), spellId, now, backoffRemaining))
            {
                continue;
            }

            ChessSquare targetSquare;
            bool targetBoardSquareFound = false;
            bool targetActiveBoardPiece = false;
            std::string validationReason;
            bool alive = false, inWorld = false, chessPiece = false, enemySide = false, isKing = false;
            if (!ValidateLiveChessBoardTargetForCast(bot, board, enemyKing, true, validationReason, alive, inWorld, chessPiece, enemySide, isKing, targetSquare, targetBoardSquareFound, targetActiveBoardPiece))
            {
                continue;
            }

            if (IsChessHealSpellBlockedOnEnemy(spellId))
                continue;

            if (!IsChessSpellTargetInRange(piece, enemyKing, spellId))
                continue;

            if (CastOffensiveChessSpell(piece, enemyKing, spellId))
            {
                ClearChessSpellNoOpBackoff(piece->GetGUID(), spellId);
                StampChessPieceCommandGcd(piece);
                chessLastAbilityCommandByPiece[piece->GetGUID()] = now;
                LogChessDecision(bot, piece, "attack-cast", "spell=" + std::to_string(spellId) + "/" +
                    GetChessSpellName(spellId) + " shape=" + GetChessSpellShapeLabel(spellId) + " target=" +
                    enemyKing->GetName() + " distance=" + std::to_string(piece->GetExactDist2d(enemyKing)) +
                    " effective_range=" + std::to_string(GetChessOffensiveSpellRange(piece, spellId)) +
                    " target_kind=king", true);
                return true;
            }

            LogChessDecision(bot, piece, "attack-cast-failed", "spell=" + std::to_string(spellId) +
                " target=" + enemyKing->GetName() + " distance=" +
                std::to_string(piece->GetExactDist2d(enemyKing)), true);
            StampChessSpellNoOpBackoff(piece->GetGUID(), spellId, now, 2);
            continue;
        }
    }

    return false;
}

bool KarazhanChessBlockEnemyPathAction::Execute(Event /*event*/)
{
    Creature* piece = bot->GetCharm() ? bot->GetCharm()->ToCreature() : nullptr;
    if (!piece || !IsChessPieceEntry(piece->GetEntry()))
        return false;
    if (!IsPawnEntry(piece->GetEntry()))
        return false;
    if (!IsChessEncounterRelevant(botAI, bot))
        return false;
    if (!IsChessPieceCommandGcdReady(piece))
        return false;
    const time_t now = std::time(nullptr);
    const uint32 instanceId = bot->GetMap() ? bot->GetMap()->GetInstanceId() : 0;

    Creature* king = GetFriendlyChessKing(botAI, bot);
    Creature* enemyKing = GetEnemyChessKing(botAI, bot);
    if (!king || !enemyKing)
        return false;

    float midX = (king->GetPositionX() + enemyKing->GetPositionX()) * 0.5f;
    float midY = (king->GetPositionY() + enemyKing->GetPositionY()) * 0.5f;

    ChessBoardState board = BuildChessBoardState(botAI, bot);
    ChessPhase phase = GetChessPhase(botAI, bot, board);
    std::string confirmSource;
    ChessPendingMoveResult const pendingResult = TryResolvePendingChessMove(botAI, bot, piece, board, now, confirmSource);
    if (pendingResult == ChessPendingMoveResult::CONFIRMED || pendingResult == ChessPendingMoveResult::WAITING || pendingResult == ChessPendingMoveResult::EXPIRED)
        return true;

    if (!IsChessMoveReady(piece, now, 5))
        return false;

    if (phase == ChessPhase::OPENING && HasPawnMovedDuringOpening(instanceId, piece->GetGUID()))
        return false;
    auto retryIt = chessOpeningMoveRetryUntilByPiece.find(piece->GetGUID());
    if (phase == ChessPhase::OPENING && retryIt != chessOpeningMoveRetryUntilByPiece.end() && now < retryIt->second)
        return false;
    ChessSquare oldSq;
    auto oldSqIt = chessLastSquareByPiece.find(piece->GetGUID());
    if (oldSqIt != chessLastSquareByPiece.end())
        oldSq = oldSqIt->second;
    std::string legalDbg;
    std::vector<Creature*> triggers = GetLegalMoveTriggersForPiece(botAI, bot, piece, board, legalDbg);
    uint32 const moveSpell = GetChessMoveSpellForPiece(piece);
    Creature* best = nullptr;
    float bestDist = std::numeric_limits<float>::max();
    for (Creature* trigger : triggers)
    {
        if (piece->GetExactDist2d(trigger) > 30.0f)
            continue;
        int candidateRow = -1;
        int candidateCol = -1;
        ObjectGuid reservedBy;
        if (!WorldToChessSquare(board, trigger->GetPositionX(), trigger->GetPositionY(), candidateRow, candidateCol) ||
            IsChessDestinationReserved(instanceId, ChessSquare{ candidateRow, candidateCol }, now,
                piece->GetGUID(), reservedBy))
            continue;
        if (IsChessMoveTriggerBackedOff(piece->GetGUID(), trigger->GetGUID(), now))
            continue;
        float d = trigger->GetExactDist2d(midX, midY);
        if (d < bestDist)
        {
            bestDist = d;
            best = trigger;
        }
    }

    if (!best)
        return false;

    if (!moveSpell || piece->HasSpellCooldown(moveSpell))
    {
        return false;
    }

    int toRow = -1;
    int toCol = -1;
    if (!WorldToChessSquare(board, best->GetPositionX(), best->GetPositionY(), toRow, toCol) ||
        !IsInsideBoard(board, toRow, toCol))
    {
        return false;
    }

    SpellCastResult const moveResult = piece->CastSpell(best, moveSpell, true);
    if (!CompleteChessSpellCast(piece, moveSpell, moveResult))
    {
        chessLastFailedMoveTriggerByPiece[piece->GetGUID()] = best->GetGUID();
        chessLastFailedMoveTimeByPiece[piece->GetGUID()] = now;
        BackoffChessMoveTrigger(piece->GetGUID(), best->GetGUID(), now);
        LogChessDecision(bot, piece, "move-cast-failed", "spell=" + std::to_string(moveSpell) +
            " source=block-path destination=" + ChessSquareToString(ChessSquare{ toRow, toCol }) +
            " cast_result=" + std::to_string(static_cast<uint32>(moveResult)));
        return false;
    }

    RecordPendingChessMove(
        bot, piece, instanceId, oldSq, ChessSquare{ toRow, toCol }, moveSpell, best->GetGUID(), "block-path", now, 8);
    return true;
}

bool KarazhanChessReleaseOrReassignAction::Execute(Event /*event*/)
{
    if (!IsChessEncounterRelevant(botAI, bot))
        return false;

    if (!IsChessEventActive(botAI, bot))
    {
        if (Creature* controlled = bot->GetCharm() ? bot->GetCharm()->ToCreature() : nullptr)
            chessSelfAbilityThrottleByPiece.erase(controlled->GetGUID());
        chessLastEnemyKingSquareByBot.erase(bot->GetGUID());
        if (bot->GetMap())
        {
            uint32 const instanceId = bot->GetMap()->GetInstanceId();
            chessEventStartByInstance.erase(instanceId);
            chessOpenedLanePawnsByInstance.erase(instanceId);
            chessOpeningProgressConfirmedByInstance.erase(instanceId);
        }
        if (Creature* controlled = bot->GetCharm() ? bot->GetCharm()->ToCreature() : nullptr)
        {
            ObjectGuid pg = controlled->GetGUID();
            chessPendingMoveByPiece.erase(pg);
            chessLastSquareByPiece.erase(pg);
            chessLastMoveFromSquareByPiece.erase(pg);
            chessLastMoveToSquareByPiece.erase(pg);
            chessLastMoveCommandByPiece.erase(pg);
            chessLastAbilityCommandByPiece.erase(pg);
            chessOffensiveTargetByPiece.erase(pg);
            chessMovementCooldownUntilByPiece.erase(pg);
            chessFailedMoveTriggerUntilByPiece.erase(pg);
            chessAbilityNoOpBackoffByPiece.erase(pg);
        }
        ClearPendingChessClaim(bot, "event-inactive");
        ClearAssignedChessPiece(bot);
        return false;
    }

    CleanupStaleChessState(botAI, bot, "release-or-reassign");

    Creature* charm = bot->GetCharm() ? bot->GetCharm()->ToCreature() : nullptr;
    if (charm && !IsChessPieceEntry(charm->GetEntry()))
    {
        chessSelfAbilityThrottleByPiece.erase(charm->GetGUID());
        chessPendingMoveByPiece.erase(charm->GetGUID());
        chessOffensiveTargetByPiece.erase(charm->GetGUID());
        chessAbilityNoOpBackoffByPiece.erase(charm->GetGUID());
        chessLastEnemyKingSquareByBot.erase(bot->GetGUID());
        ClearPendingChessClaim(bot, "non-chess-charm");
        ClearAssignedChessPiece(bot);
        return false;
    }
    if (charm && IsChessPieceEntry(charm->GetEntry()))
    {
        ChessBoardState board = BuildChessBoardState(botAI, bot);
        ChessSquare controlledSq;
        std::string controlledRejectReason;
        if (!IsActiveBoardFriendlyControlledChessPiece(bot, board, charm, controlledSq, controlledRejectReason))
        {
            HandleInvalidControlledChessPiece(botAI, bot, charm, board, "release_or_reassign", controlledRejectReason);
            return false;
        }
    }

    Creature* assigned = GetAssignedChessPiece(bot);
    if (assigned && (!assigned->IsAlive() || assigned->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE)))
    {
        time_t const now = std::time(nullptr);
        SuppressReclaimForPiece(assigned->GetGUID(), now, 20);
        chessSelfAbilityThrottleByPiece.erase(assigned->GetGUID());
        chessPendingMoveByPiece.erase(assigned->GetGUID());
        chessOffensiveTargetByPiece.erase(assigned->GetGUID());
        chessAbilityNoOpBackoffByPiece.erase(assigned->GetGUID());
        ClearPendingChessClaim(bot, "assigned-invalid");
        ClearAssignedChessPiece(bot);
    }
    else if (assigned)
    {
        bool const openingProgressConfirmed = HasConfirmedOpeningProgress(bot->GetMap() ? bot->GetMap()->GetInstanceId() : 0);
        bool const pawnOnlyMode = !openingProgressConfirmed;
        bool const isPawn = IsPawnEntry(assigned->GetEntry());
        bool const isFriendlyForBot = IsFriendlyChessPieceForBot(bot, assigned);
        bool const notSelectable = assigned->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        bool const assignedToOtherBot = IsPieceAssignedToOtherBot(bot, assigned);

        bool const pawnRetireCandidate = openingProgressConfirmed && isPawn &&
            HasPawnMovedDuringOpening(bot->GetMap() ? bot->GetMap()->GetInstanceId() : 0, assigned->GetGUID()) &&
            HasAvailableNonPawnChessPiece(botAI, bot, openingProgressConfirmed);

        if (pawnRetireCandidate)
        {
            time_t const now = std::time(nullptr);
            SuppressReclaimForPiece(assigned->GetGUID(), now, 20);
            if (Creature* controlled = bot->GetCharm() ? bot->GetCharm()->ToCreature() : nullptr)
            {
                if (controlled->GetGUID() == assigned->GetGUID())
                {
                    controlled->RemoveCharmedBy(bot);
                }
            }
            chessSelfAbilityThrottleByPiece.erase(assigned->GetGUID());
            chessPendingMoveByPiece.erase(assigned->GetGUID());
            chessOffensiveTargetByPiece.erase(assigned->GetGUID());
            chessAbilityNoOpBackoffByPiece.erase(assigned->GetGUID());
            chessLastAbilityCommandByPiece.erase(assigned->GetGUID());
            chessLastMoveCommandByPiece.erase(assigned->GetGUID());
            chessMovementCooldownUntilByPiece.erase(assigned->GetGUID());
            chessLastFailedMoveTriggerByPiece.erase(assigned->GetGUID());
            chessLastFailedMoveTimeByPiece.erase(assigned->GetGUID());
            chessFailedMoveTriggerUntilByPiece.erase(assigned->GetGUID());
            chessOpeningMoveRetryUntilByPiece.erase(assigned->GetGUID());
            ClearPendingChessClaim(bot, "pawn-retired-for-upgrade");
            ClearAssignedChessPiece(bot);
            return false;
        }
        bool const openingAssignedAllowed = pawnOnlyMode ? IsClaimableFriendlyPawnForOpening(bot, assigned) : IsClaimableChessPieceForBot(bot, assigned, openingProgressConfirmed);
        bool const assignedAllowed = openingAssignedAllowed && !assignedToOtherBot && (!pawnOnlyMode || isPawn);

        if (!assignedAllowed)
        {
            Unit* controller = assigned->GetCharmerOrOwner();
            std::string clearReason = !isFriendlyForBot ? "wrong-side" :
                (!assigned->IsAlive() ? "dead" :
                (assignedToOtherBot ? "assigned-to-other-bot" :
                (!openingProgressConfirmed && !isPawn ? "not-pawn" :
                (!openingProgressConfirmed && assigned->IsCharmed() ? "already-charmed" :
                (!openingProgressConfirmed && notSelectable ? "not-selectable" : "not-claimable")))));
            chessSelfAbilityThrottleByPiece.erase(assigned->GetGUID());
            SuppressReclaimForPiece(assigned->GetGUID(), std::time(nullptr), 20);
            chessOffensiveTargetByPiece.erase(assigned->GetGUID());
            chessAbilityNoOpBackoffByPiece.erase(assigned->GetGUID());
            ClearPendingChessClaim(bot, "assigned-not-allowed");
            ClearAssignedChessPiece(bot);
        }
    }

    return false;
}
