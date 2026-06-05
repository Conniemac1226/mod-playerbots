#include "KaraActions.h"
#include "KaraHelpers.h"
#include "Playerbots.h"
#include "PlayerbotTextMgr.h"
#include "RaidBossHelpers.h"
#include "RtiTargetValue.h"
#include "MotionMaster.h"
#include <algorithm>
#include <cctype>
#include <ctime>
#include <limits>
#include <unordered_map>
#include <unordered_set>

using namespace KarazhanHelpers;

// Trash

// Mana Warps blow up when they die for massive raid damage
// But they cannot cast the ability if they are stunned
bool ManaWarpStunCreatureBeforeWarpBreachAction::Execute(Event /*event*/)
{
    Unit* manaWarp = GetFirstAliveUnitByEntry(botAI, NPC_MANA_WARP);
    if (!manaWarp)
        return false;

    static const std::array<const char*, 8> spells =
    {
        "bash",
        "concussion blow",
        "hammer of justice",
        "kidney shot",
        "maim",
        "revenge stun",
        "shadowfury",
        "shockwave"
    };

    for (const char* spell : spells)
    {
        if (botAI->CanCastSpell(spell, manaWarp))
            return botAI->CastSpell(spell, manaWarp);
    }

    return false;
}

// Attumen the Huntsman

// Prioritize Midnight until Attumen is mounted
bool AttumenTheHuntsmanMarkTargetAction::Execute(Event /*event*/)
{
    Unit* attumenMounted = GetFirstAliveUnitByEntry(botAI, NPC_ATTUMEN_THE_HUNTSMAN_MOUNTED);
    if (attumenMounted)
    {
        if (IsMechanicTrackerBot(botAI, bot, KARAZHAN_MAP_ID, nullptr))
            MarkTargetWithStar(bot, attumenMounted);

        SetRtiTarget(botAI, "star", attumenMounted);

        if (AI_VALUE(Unit*, "current target") != attumenMounted)
            return Attack(attumenMounted);

    }
    else if (Unit* midnight = AI_VALUE2(Unit*, "find target", "midnight"))
    {
        if (IsMechanicTrackerBot(botAI, bot, KARAZHAN_MAP_ID, nullptr))
            MarkTargetWithStar(bot, midnight);

        if (!botAI->IsAssistTankOfIndex(bot, 0))
        {
            SetRtiTarget(botAI, "star", midnight);

            if (AI_VALUE(Unit*, "current target") != midnight)
                return Attack(midnight);
        }
    }

    return false;
}

// Off tank should move Attumen out of the way so he doesn't cleave bots
bool AttumenTheHuntsmanSplitBossesAction::Execute(Event /*event*/)
{
    Unit* midnight = AI_VALUE2(Unit*, "find target", "midnight");
    if (!midnight)
        return false;

    Unit* attumen = GetFirstAliveUnitByEntry(botAI, NPC_ATTUMEN_THE_HUNTSMAN);
    if (!attumen)
        return false;

    MarkTargetWithSquare(bot, attumen);
    SetRtiTarget(botAI, "square", attumen);

    if (AI_VALUE(Unit*, "current target") != attumen)
        return Attack(attumen);

    if (attumen->GetVictim() == bot && midnight->GetVictim() != bot)
    {
        const float safeDistance = 6.0f;
        Unit* nearestPlayer = GetNearestPlayerInRadius(bot, safeDistance);
        if (nearestPlayer && attumen->GetExactDist2d(nearestPlayer) < safeDistance)
            return MoveFromGroup(safeDistance + 2.0f);
    }

    return false;
}

// Stack behind mounted Attumen (inside minimum range of Berserker Charge)
bool AttumenTheHuntsmanStackBehindAction::Execute(Event /*event*/)
{
    Unit* attumenMounted = GetFirstAliveUnitByEntry(botAI, NPC_ATTUMEN_THE_HUNTSMAN_MOUNTED);
    if (!attumenMounted)
        return false;

    const float distanceBehind = botAI->IsRanged(bot) ? 6.0f : 2.0f;
    float orientation = attumenMounted->GetOrientation() + M_PI;
    float rearX = attumenMounted->GetPositionX() + std::cos(orientation) * distanceBehind;
    float rearY = attumenMounted->GetPositionY() + std::sin(orientation) * distanceBehind;

    if (bot->GetDistance2d(rearX, rearY) > 1.0f)
    {
        return MoveTo(KARAZHAN_MAP_ID, rearX, rearY, bot->GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

// Reset timer for bots to pause DPS when Attumen mounts Midnight
bool AttumenTheHuntsmanManageDpsTimerAction::Execute(Event /*event*/)
{
    Unit* midnight = AI_VALUE2(Unit*, "find target", "midnight");
    if (!midnight)
        return false;

    const uint32 instanceId = midnight->GetMap()->GetInstanceId();

    if (midnight && midnight->GetHealth() == midnight->GetMaxHealth())
        attumenDpsWaitTimer.erase(instanceId);

    // Midnight is still present as a separate (invisible) unit after Attumen mounts
    // So this block can be reached
    Unit* attumenMounted = GetFirstAliveUnitByEntry(botAI, NPC_ATTUMEN_THE_HUNTSMAN_MOUNTED);
    if (!attumenMounted)
        return false;

    const time_t now = std::time(nullptr);

    if (attumenMounted)
        attumenDpsWaitTimer.try_emplace(instanceId, now);

    return false;
}

// Moroes

bool MoroesMainTankAttackBossAction::Execute(Event /*event*/)
{
    Unit* moroes = AI_VALUE2(Unit*, "find target", "moroes");
    if (!moroes)
        return false;

    MarkTargetWithCircle(bot, moroes);
    SetRtiTarget(botAI, "circle", moroes);

    if (AI_VALUE(Unit*, "current target") != moroes)
        return Attack(moroes);

    return false;
}

// Mark targets with skull in the recommended kill order
bool MoroesMarkTargetAction::Execute(Event /*event*/)
{
    Unit* dorothea = AI_VALUE2(Unit*, "find target", "baroness dorothea millstipe");
    Unit* catriona = AI_VALUE2(Unit*, "find target", "lady catriona von'indi");
    Unit* keira = AI_VALUE2(Unit*, "find target", "lady keira berrybuck");
    Unit* rafe = AI_VALUE2(Unit*, "find target", "baron rafe dreuger");
    Unit* robin = AI_VALUE2(Unit*, "find target", "lord robin daris");
    Unit* crispin = AI_VALUE2(Unit*, "find target", "lord crispin ference");
    Unit* target = GetFirstAliveUnit({dorothea, catriona, keira, rafe, robin, crispin});

    if (target)
    {
        if (IsMechanicTrackerBot(botAI, bot, KARAZHAN_MAP_ID, nullptr))
            MarkTargetWithSkull(bot, target);

        SetRtiTarget(botAI, "skull", target);
    }

    return false;
}

// Maiden of Virtue

// Tank the boss in the center of the room
// Move to healers after Repentenace to break the stun
bool MaidenOfVirtueMoveBossToHealerAction::Execute(Event /*event*/)
{
    Unit* maiden = AI_VALUE2(Unit*, "find target", "maiden of virtue");
    if (!maiden)
        return false;

    if (AI_VALUE(Unit*, "current target") != maiden)
        return Attack(maiden);

    Unit* healer = nullptr;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || !botAI->IsHeal(member) ||
                !member->HasAura(SPELL_REPENTANCE))
                continue;

            healer = member;
            break;
        }
    }

    if (healer)
    {
        float angle = healer->GetOrientation();
        float targetX = healer->GetPositionX() + std::cos(angle) * 6.0f;
        float targetY = healer->GetPositionY() + std::sin(angle) * 6.0f;
        {
            return MoveTo(KARAZHAN_MAP_ID, targetX, targetY, healer->GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    const Position& position = MAIDEN_OF_VIRTUE_BOSS_POSITION;
    const float maxDistance = 2.0f;
    float distanceToPosition = maiden->GetExactDist2d(position);
    if (distanceToPosition > maxDistance)
    {
        float dX = position.GetPositionX() - maiden->GetPositionX();
        float dY = position.GetPositionY() - maiden->GetPositionY();
        float mX = position.GetPositionX() + (dX / distanceToPosition) * maxDistance;
        float mY = position.GetPositionY() + (dY / distanceToPosition) * maxDistance;
        {
            return MoveTo(KARAZHAN_MAP_ID, mX, mY, position.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

// Spread out ranged DPS between the pillars
bool MaidenOfVirtuePositionRangedAction::Execute(Event /*event*/)
{
    const uint8 maxIndex = 7;
    uint8 index = 0;

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !botAI->IsRanged(member))
                continue;

            if (member == bot)
                break;

            if (index >= maxIndex)
            {
                index = 0;
                continue;
            }
            index++;
        }
    }

    const Position& position = MAIDEN_OF_VIRTUE_RANGED_POSITION[index];
    if (bot->GetExactDist2d(position) > 2.0f)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return MoveTo(KARAZHAN_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
                      false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

// The Big Bad Wolf

// Tank the boss at the front left corner of the stage
bool BigBadWolfPositionBossAction::Execute(Event /*event*/)
{
    Unit* wolf = AI_VALUE2(Unit*, "find target", "the big bad wolf");
    if (!wolf)
        return false;

    if (AI_VALUE(Unit*, "current target") != wolf)
        return Attack(wolf);

    if (wolf->GetVictim() == bot)
    {
        const Position& position = BIG_BAD_WOLF_BOSS_POSITION;
        float distanceToPosition = wolf->GetExactDist2d(position);

        if (distanceToPosition > 2.0f)
        {
            float dX = position.GetPositionX() - wolf->GetPositionX();
            float dY = position.GetPositionY() - wolf->GetPositionY();
            float moveDist = std::min(5.0f, distanceToPosition);
            float moveX = wolf->GetPositionX() + (dX / distanceToPosition) * moveDist;
            float moveY = wolf->GetPositionY() + (dY / distanceToPosition) * moveDist;

            return MoveTo(KARAZHAN_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

// Run away, little girl, run away
bool BigBadWolfRunAwayFromBossAction::Execute(Event /*event*/)
{
    const ObjectGuid botGuid = bot->GetGUID();
    uint8 index = bigBadWolfRunIndex.count(botGuid) ? bigBadWolfRunIndex[botGuid] : 0;

    while (bot->GetExactDist2d(BIG_BAD_WOLF_RUN_POSITION[index].GetPositionX(),
                               BIG_BAD_WOLF_RUN_POSITION[index].GetPositionY()) < 1.0f)
    {
        index = (index + 1) % 4;
    }

    bigBadWolfRunIndex[botGuid] = index;
    bot->AttackStop();
    bot->InterruptNonMeleeSpells(true);

    const Position& position = BIG_BAD_WOLF_RUN_POSITION[index];
    return MoveTo(KARAZHAN_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
                  false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
}

// Romulo and Julianne

// Keep the couple within 10% HP of each other
bool RomuloAndJulianneMarkTargetAction::Execute(Event /*event*/)
{
    Unit* romulo = AI_VALUE2(Unit*, "find target", "romulo");
    if (!romulo)
        return false;

    Unit* julianne = AI_VALUE2(Unit*, "find target", "julianne");
    if (!julianne)
        return false;

    Unit* target = nullptr;
    const float maxPctDifference = 10.0f;

    if (julianne->GetHealthPct() + maxPctDifference < romulo->GetHealthPct() || julianne->GetHealthPct() < 1.0f)
        target = romulo;
    else if (romulo->GetHealthPct() + maxPctDifference < julianne->GetHealthPct() || romulo->GetHealthPct() < 1.0f)
        target = julianne;
    else
        target = (romulo->GetHealthPct() >= julianne->GetHealthPct()) ? romulo : julianne;

    if (target)
        MarkTargetWithSkull(bot, target);

    return false;
}

// The Wizard of Oz

// Mark targets with skull in the recommended kill order
bool WizardOfOzMarkTargetAction::Execute(Event /*event*/)
{
    Unit* dorothee = AI_VALUE2(Unit*, "find target", "dorothee");
    Unit* tito = AI_VALUE2(Unit*, "find target", "tito");
    Unit* roar = AI_VALUE2(Unit*, "find target", "roar");
    Unit* strawman = AI_VALUE2(Unit*, "find target", "strawman");
    Unit* tinhead = AI_VALUE2(Unit*, "find target", "tinhead");
    Unit* crone = AI_VALUE2(Unit*, "find target", "the crone");
    Unit* target = GetFirstAliveUnit({dorothee, tito, roar, strawman, tinhead, crone});

    if (target)
        MarkTargetWithSkull(bot, target);

    return false;
}

// Mages spam Scorch on Strawman to disorient him
bool WizardOfOzScorchStrawmanAction::Execute(Event /*event*/)
{
    Unit* strawman = AI_VALUE2(Unit*, "find target", "strawman");
    if (strawman && botAI->CanCastSpell("scorch", strawman))
        return botAI->CastSpell("scorch", strawman);

    return false;
}

// The Curator

// Prioritize destroying Astral Flares
bool TheCuratorMarkAstralFlareAction::Execute(Event /*event*/)
{
    Unit* flare = AI_VALUE2(Unit*, "find target", "astral flare");
    if (!flare)
        return false;

    if (IsMechanicTrackerBot(botAI, bot, KARAZHAN_MAP_ID, nullptr))
        MarkTargetWithSkull(bot, flare);

    SetRtiTarget(botAI, "skull", flare);

    return false;
}

// Tank the boss in the center of the hallway near the Guardian's Library
// Main tank and off tank will attack the boss; others will focus on Astral Flares
bool TheCuratorPositionBossAction::Execute(Event /*event*/)
{
    Unit* curator = AI_VALUE2(Unit*, "find target", "the curator");
    if (!curator)
        return false;

    MarkTargetWithCircle(bot, curator);
    SetRtiTarget(botAI, "circle", curator);

    if (AI_VALUE(Unit*, "current target") != curator)
        return Attack(curator);

    if (curator->GetVictim() == bot)
    {
        const Position& position = THE_CURATOR_BOSS_POSITION;
        float distanceToPosition = curator->GetExactDist2d(position);

        if (distanceToPosition > 2.0f)
        {
            float dX = position.GetPositionX() - curator->GetPositionX();
            float dY = position.GetPositionY() - curator->GetPositionY();
            float moveDist = std::min(10.0f, distanceToPosition);
            float moveX = position.GetPositionX() + (dX / distanceToPosition) * moveDist;
            float moveY = position.GetPositionY() + (dY / distanceToPosition) * moveDist;

            return MoveTo(KARAZHAN_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

// Spread out ranged DPS to avoid Arcing Sear damage
bool TheCuratorSpreadRangedAction::Execute(Event /*event*/)
{
    const float minDistance = 5.0f;
    Unit* nearestPlayer = GetNearestPlayerInRadius(bot, minDistance);

    if (nearestPlayer)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return FleePosition(nearestPlayer->GetPosition(), minDistance);
    }

    return false;
}

// Terestian Illhoof

// Prioritize (1) Demon Chains, (2) Kil'rek, (3) Illhoof
bool TerestianIllhoofMarkTargetAction::Execute(Event /*event*/)
{
    Unit* demonChains = GetFirstAliveUnitByEntry(botAI, NPC_DEMON_CHAINS);
    Unit* kilrek = GetFirstAliveUnitByEntry(botAI, NPC_KILREK);
    Unit* illhoof = AI_VALUE2(Unit*, "find target", "terestian illhoof");

    Unit* target = GetFirstAliveUnit({demonChains, kilrek, illhoof});
    if (target)
        MarkTargetWithSkull(bot, target);

    return false;
}

// Shade of Aran

// Run to the edge of the room to avoid Arcane Explosion
bool ShadeOfAranRunAwayFromArcaneExplosionAction::Execute(Event /*event*/)
{
    Unit* aran = AI_VALUE2(Unit*, "find target", "shade of aran");
    if (!aran)
        return false;

    const float safeDistance = 20.0f;
    float distance = bot->GetDistance2d(aran);
    if (distance < safeDistance)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return MoveAway(aran, safeDistance - distance);
    }

    return false;
}

// I will not move when Flame Wreath is cast or the raid blows up
bool ShadeOfAranStopMovingDuringFlameWreathAction::Execute(Event /*event*/)
{
    AI_VALUE(LastMovement&, "last movement").Set(nullptr);

    if (bot->isMoving())
    {
        bot->GetMotionMaster()->Clear();
        bot->StopMoving();
        return true;
    }

    return false;
}

// Mark Conjured Elementals with skull so DPS can burn them down
bool ShadeOfAranMarkConjuredElementalAction::Execute(Event /*event*/)
{
    Unit* elemental = GetFirstAliveUnitByEntry(botAI, NPC_CONJURED_ELEMENTAL);

    if (elemental)
        MarkTargetWithSkull(bot, elemental);

    return false;
}

// Don't get closer than 11 yards to Aran to avoid counterspell
// Don't get farther than 15 yards from Aran to avoid getting stuck in alcoves
bool ShadeOfAranRangedMaintainDistanceAction::Execute(Event /*event*/)
{
    Unit* aran = AI_VALUE2(Unit*, "find target", "shade of aran");
    if (!aran)
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    const float minDist = 11.0f;
    const float maxDist = 15.0f;
    const float ringIncrement = M_PI / 8;
    const float distIncrement = 0.5f;

    float bestX = 0, bestY = 0, bestMoveDist = std::numeric_limits<float>::max();
    bool found = false;

    for (float dist = minDist; dist <= maxDist; dist += distIncrement)
    {
        for (float angle = 0; angle < 2 * M_PI; angle += ringIncrement)
        {
            float x = aran->GetPositionX() + std::cos(angle) * dist;
            float y = aran->GetPositionY() + std::sin(angle) * dist;

            bool tooClose = false;
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (!member || member == bot || !member->IsAlive())
                    continue;
                if (member->GetExactDist2d(x, y) < 3.0f)
                {
                    tooClose = true;
                    break;
                }
            }
            if (tooClose)
                continue;

            float moveDist = bot->GetExactDist2d(x, y);
            if (moveDist < bestMoveDist)
            {
                bestMoveDist = moveDist;
                bestX = x;
                bestY = y;
                found = true;
            }
        }
    }

    if (found && bestMoveDist > 0.5f)
    {
        return MoveTo(KARAZHAN_MAP_ID, bestX, bestY, bot->GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

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
        std::string source;
    };

    struct ChessPendingClaim
    {
        ObjectGuid pieceGuid;
        uint32 instanceId = 0;
        time_t createdAt = 0;
        time_t expiresAt = 0;
        bool controlAttempted = false;
    };

    struct ChessAbilityNoOpBackoff
    {
        time_t until = 0;
    };

    std::unordered_map<ObjectGuid, ObjectGuid> chessLastFailedMoveTriggerByPiece;
    std::unordered_map<ObjectGuid, time_t> chessLastFailedMoveTimeByPiece;
    std::unordered_map<ObjectGuid, time_t> chessLastMoveCommandByPiece;
    std::unordered_map<ObjectGuid, time_t> chessLastAbilityCommandByPiece;
    std::unordered_map<ObjectGuid, uint32> chessLastAnyCommandMsByPiece;
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
    std::unordered_map<ObjectGuid, uint8> chessPawnOpeningAdvanceCountByPiece;
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

    static uint8 GetPawnOpeningAdvanceCount(ObjectGuid const& pieceGuid)
    {
        auto it = chessPawnOpeningAdvanceCountByPiece.find(pieceGuid);
        return it != chessPawnOpeningAdvanceCountByPiece.end() ? it->second : 0;
    }

    static void IncrementPawnOpeningAdvanceCount(ObjectGuid const& pieceGuid)
    {
        if (!pieceGuid)
            return;

        uint8& count = chessPawnOpeningAdvanceCountByPiece[pieceGuid];
        if (count < std::numeric_limits<uint8>::max())
            ++count;
    }

    static bool IsCentralChessPawnFile(ChessSquare const& square)
    {
        return square.col >= 2 && square.col <= 5;
    }

    static bool IsPawnEntry(uint32 e);
    static bool IsKingEntry(uint32 e);
    static bool IsSummonedDaemonChessPiece(uint32 entry);
    static bool IsOrcWarlockChessPiece(uint32 entry);
    static void PurgeChessPieceCacheForGuid(Player* bot, ObjectGuid const& pieceGuid, std::string const& source);

    static bool IsInsideBoard(ChessBoardState const& b, int row, int col);
    static bool WorldToChessSquare(ChessBoardState const& s, float x, float y, int& row, int& col);

    static bool HasClearBacklineRoute(ChessBoardState const& board, ChessSquare const& pawnSquare, int forward)
    {
        int retreat = -forward;
        if (retreat == 0)
            retreat = 1;

        for (int step = 1; step <= 2; ++step)
        {
            int row = pawnSquare.row + (retreat * step);
            int col = pawnSquare.col;
            if (!IsInsideBoard(board, row, col))
                return false;
            if (board.occupied.find({ row, col }) != board.occupied.end())
                return false;
        }

        return true;
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

    static bool IsHellfireChessAoeSpell(uint32 spellId)
    {
        return spellId == 37428;
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
               reason == "no_enemy_candidate" ||
               reason == "no_valid_candidate";
    }

    static bool IsChessSupportSpell(uint32 spellId, std::string& rejectReason)
    {
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

    static bool IsChessSupportSpellForNonKingTarget(uint32 spellId, std::string& rejectReason)
    {
        if (spellId == 37456) // Shadow Mend applies to king-style support targets, but repeatedly fails on non-king pieces.
        {
            rejectReason = "necrolyte_shadow_mend_nonking_unsupported";
            return false;
        }

        return IsChessSupportSpell(spellId, rejectReason);
    }

    static ChessSupportTargetSelection SelectDamagedFriendlyActiveBoardNonKingTarget(Player* bot, ChessBoardState const& board, Creature* piece)
    {
        ChessSupportTargetSelection selection;
        if (!bot || !piece)
        {
            selection.rejectReason = "invalid_context";
            return selection;
        }

        bool sawFriendly = false;
        bool sawDamaged = false;
        for (auto const& boardEntry : board.pieceSquare)
        {
            Creature* candidate = ObjectAccessor::GetCreature(*piece, boardEntry.first);
            if (!candidate || !candidate->IsInWorld() || !candidate->IsAlive() || !IsChessPieceEntry(candidate->GetEntry()) ||
                !IsFriendlyChessPieceForBot(bot, candidate) || IsKingChessPieceEntry(candidate->GetEntry()))
                continue;

            auto occupiedIt = board.occupied.find({ boardEntry.second.row, boardEntry.second.col });
            if (occupiedIt == board.occupied.end())
                continue;

            sawFriendly = true;
            float const hpPct = candidate->GetHealthPct();
            if (hpPct >= 90.0f)
                continue;

            sawDamaged = true;
            float const distance = piece->GetExactDist2d(candidate);
            if (!selection.target || hpPct < selection.healthPct || (hpPct == selection.healthPct && distance < selection.distance))
            {
                selection.target = candidate;
                selection.healthPct = hpPct;
                selection.distance = distance;
                selection.rejectReason = "none";
            }
        }

        if (!selection.target)
            selection.rejectReason = sawDamaged ? "no_castable_support_target" : (sawFriendly ? "no_damaged_friendly" : "no_friendly_active_board_piece");

        return selection;
    }

    static ChessOffensiveTargetSelection SelectNonKingChessTarget(PlayerbotAI* botAI, Player* bot, Creature* piece, ChessBoardState const& board, char const* source)
    {
        ChessOffensiveTargetSelection selection;
        if (!botAI || !bot || !piece)
        {
            selection.rejectReason = "invalid_context";
            return selection;
        }

        constexpr float maxTargetRange = 30.0f;
        Creature* bestSupport = nullptr;
        Creature* bestDamage = nullptr;
        Creature* bestPawn = nullptr;
        float bestSupportDistance = std::numeric_limits<float>::max();
        float bestDamageDistance = std::numeric_limits<float>::max();
        float bestPawnDistance = std::numeric_limits<float>::max();
        bool sawEnemy = false;
        bool sawInRange = false;
        bool sawOutOfRange = false;
        bool sawInactive = false;
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
            if (distance > maxTargetRange)
            {
                sawOutOfRange = true;
                continue;
            }

            sawInRange = true;

            if (IsHealerChessPieceEntry(candidate->GetEntry()))
            {
                if (!bestSupport || distance < bestSupportDistance)
                {
                    bestSupport = candidate;
                    bestSupportDistance = distance;
                }
                continue;
            }

            if (IsDamageChessPieceEntry(candidate->GetEntry()))
            {
                if (!bestDamage || distance < bestDamageDistance)
                {
                    bestDamage = candidate;
                    bestDamageDistance = distance;
                }
                continue;
            }

            if (IsPawnEntry(candidate->GetEntry()))
            {
                if (!bestPawn || distance < bestPawnDistance)
                {
                    bestPawn = candidate;
                    bestPawnDistance = distance;
                }
                continue;
            }
        }

        if (bestSupport)
        {
            selection.target = bestSupport;
            selection.category = "support";
            selection.distance = bestSupportDistance;
            selection.rejectReason = "none";
            return selection;
        }

        if (bestDamage)
        {
            selection.target = bestDamage;
            selection.category = "damage";
            selection.distance = bestDamageDistance;
            selection.rejectReason = "none";
            return selection;
        }

        if (bestPawn)
        {
            selection.target = bestPawn;
            selection.category = "pawn";
            selection.distance = bestPawnDistance;
            selection.rejectReason = "none";
            return selection;
        }

        if (!sawEnemy)
            selection.rejectReason = "no_enemy_candidate";
        else if (sawOutOfRange && !sawInRange)
            selection.rejectReason = "out_of_range";
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

    static float GetChessPracticalAttackRange(Creature* piece)
    {
        if (!piece)
            return 22.0f;

        if (IsSummonedDaemonChessPiece(piece->GetEntry()))
            return 10.0f;

        float bestRange = 0.0f;
        for (uint8 i = 0; i < MAX_CREATURE_SPELLS; ++i)
        {
            uint32 const spellId = piece->m_spells[i];
            if (!spellId)
                continue;

            std::string rejectReason;
            if (!IsKingAttackOffensiveChessSpell(spellId, rejectReason))
                continue;

            if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId))
                bestRange = std::max(bestRange, spellInfo->GetMaxRange(false));
        }

        if (IsOrcWarlockChessPiece(piece->GetEntry()))
            return bestRange > 0.0f ? bestRange : 30.0f;

        return bestRange > 0.0f ? bestRange : 22.0f;
    }

    static ChessOffensiveTargetSelection SelectSummonedDaemonChessTarget(PlayerbotAI* botAI, Player* bot, Creature* piece, ChessBoardState const& board, char const* source)
    {
        ChessOffensiveTargetSelection selection;
        if (!botAI || !bot || !piece)
        {
            selection.rejectReason = "invalid_context";
            return selection;
        }

        Creature* bestSupport = nullptr;
        Creature* bestDamage = nullptr;
        Creature* bestPawn = nullptr;
        float bestSupportDistance = std::numeric_limits<float>::max();
        float bestDamageDistance = std::numeric_limits<float>::max();
        float bestPawnDistance = std::numeric_limits<float>::max();
        bool sawEnemy = false;
        bool sawInactive = false;
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
            if (IsHealerChessPieceEntry(candidate->GetEntry()))
            {
                if (!bestSupport || distance < bestSupportDistance)
                {
                    bestSupport = candidate;
                    bestSupportDistance = distance;
                }
                continue;
            }

            if (IsDamageChessPieceEntry(candidate->GetEntry()))
            {
                if (!bestDamage || distance < bestDamageDistance)
                {
                    bestDamage = candidate;
                    bestDamageDistance = distance;
                }
                continue;
            }

            if (IsPawnEntry(candidate->GetEntry()))
            {
                if (!bestPawn || distance < bestPawnDistance)
                {
                    bestPawn = candidate;
                    bestPawnDistance = distance;
                }
                continue;
            }
        }

        if (bestSupport)
        {
            selection.target = bestSupport;
            selection.category = "support";
            selection.distance = bestSupportDistance;
            selection.rejectReason = "none";
            return selection;
        }

        if (bestDamage)
        {
            selection.target = bestDamage;
            selection.category = "damage";
            selection.distance = bestDamageDistance;
            selection.rejectReason = "none";
            return selection;
        }

        if (bestPawn)
        {
            selection.target = bestPawn;
            selection.category = "pawn";
            selection.distance = bestPawnDistance;
            selection.rejectReason = "none";
            return selection;
        }

        if (sawInactive)
            selection.rejectReason = "no_active_board_target:" + lastInactiveReason;
        else
            selection.rejectReason = sawEnemy ? "no_valid_candidate" : "no_enemy_candidate";

        if (!selection.target && sawInactive)
        {
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
        chessLastMoveCommandByPiece.erase(pieceGuid);
        chessLastAbilityCommandByPiece.erase(pieceGuid);
        chessLastAnyCommandMsByPiece.erase(pieceGuid);
        chessMovementCooldownUntilByPiece.erase(pieceGuid);
        chessOpeningMoveRetryUntilByPiece.erase(pieceGuid);
        chessReclaimSuppressedUntilByPiece.erase(pieceGuid);
        chessPawnOpeningAdvanceCountByPiece.erase(pieceGuid);
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
        collectPieceGuid(chessLastMoveCommandByPiece);
        collectPieceGuid(chessLastAbilityCommandByPiece);
        collectPieceGuid(chessLastAnyCommandMsByPiece);
        collectPieceGuid(chessMovementCooldownUntilByPiece);
        collectPieceGuid(chessOpeningMoveRetryUntilByPiece);
        collectPieceGuid(chessReclaimSuppressedUntilByPiece);
        collectPieceGuid(chessPawnOpeningAdvanceCountByPiece);
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
        pending.controlAttempted = false;
        chessPendingClaimByBot[bot->GetGUID()] = pending;
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

    static void RecordPendingChessMove(
        Player* bot, Creature* piece, uint32 instanceId, ChessSquare const& from, ChessSquare const& to,
        uint32 moveSpell, ObjectGuid const& triggerGuid, std::string const& source, time_t now, time_t expiresIn = 5)
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
        pending.source = source;
        chessPendingMoveByPiece[piece->GetGUID()] = pending;
        chessOpeningMoveRetryUntilByPiece[piece->GetGUID()] = pending.expiresAt;

    }

    static ChessPendingMoveResult TryResolvePendingChessMove(
        PlayerbotAI* botAI, Player* bot, Creature* piece, ChessBoardState const& board, time_t now, std::string& confirmSource)
    {
        (void)botAI;
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
            chessOpeningMoveRetryUntilByPiece[piece->GetGUID()] = now + 5;
            return ChessPendingMoveResult::EXPIRED;
        }

        auto sqIt = board.pieceSquare.find(piece->GetGUID());
        bool const squareMatch = sqIt != board.pieceSquare.end() &&
                                 sqIt->second.row == pending.toSquare.row &&
                                 sqIt->second.col == pending.toSquare.col;
        bool const auraMatch = piece->HasAura(SPELL_MOVE_COOLDOWN);
        bool const cooldownSignal = pending.moveSpell != 0 && piece->HasSpellCooldown(pending.moveSpell);
        Creature* expectedTrigger = nullptr;
        auto trigIt = board.squareToTrigger.find({ pending.toSquare.row, pending.toSquare.col });
        if (trigIt != board.squareToTrigger.end())
            expectedTrigger = trigIt->second;
        bool const triggerProximity = expectedTrigger && piece->GetExactDist2d(expectedTrigger) <= 3.5f;

        if (!squareMatch && !auraMatch && !cooldownSignal && !triggerProximity)
            return ChessPendingMoveResult::WAITING;

        confirmSource = squareMatch ? "square_match" :
            (auraMatch ? "move_cooldown_aura" :
            (cooldownSignal ? "cooldown_signal" : "trigger_proximity"));
        chessPendingMoveByPiece.erase(it);
        chessOpeningMoveRetryUntilByPiece.erase(piece->GetGUID());
        chessLastFailedMoveTriggerByPiece.erase(piece->GetGUID());
        chessLastFailedMoveTimeByPiece.erase(piece->GetGUID());
        chessLastMoveCommandByPiece[piece->GetGUID()] = now;
        chessMovementCooldownUntilByPiece[piece->GetGUID()] = now + 5;
        chessOpenedLanePawnsByInstance[pending.instanceId].insert(piece->GetGUID());
        chessLastMoveFromSquareByPiece[piece->GetGUID()] = pending.fromSquare;
        chessLastMoveToSquareByPiece[piece->GetGUID()] = pending.toSquare;
        chessLastSquareByPiece[piece->GetGUID()] = pending.toSquare;
        if (IsPawnEntry(piece->GetEntry()))
        {
            IncrementPawnOpeningAdvanceCount(piece->GetGUID());
        }
        MarkOpeningProgressConfirmed(
            pending.instanceId, bot, piece, pending.fromSquare, pending.toSquare,
            "pending-" + pending.source + "-" + confirmSource);
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

    std::string GetChessPieceSpellList(Creature* piece)
    {
        if (!piece)
            return "none";

        std::string out;
        for (uint8 i = 0; i < MAX_CREATURE_SPELLS; ++i)
        {
            uint32 spellId = piece->m_spells[i];
            if (!spellId)
                continue;
            if (!out.empty())
                out += ",";
            out += std::to_string(spellId);
        }
        return out.empty() ? "none" : out;
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

    static void AppendRayMoves(ChessBoardState const& b, int row, int col, int dr, int dc,
                               std::set<std::pair<int, int>>& out)
    {
        int r = row + dr;
        int c = col + dc;
        while (IsInsideBoard(b, r, c))
        {
            if (b.occupied.find({ r, c }) != b.occupied.end())
                break;
            out.insert({ r, c });
            r += dr;
            c += dc;
        }
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
        Creature* enemyKing = nullptr;
        for (auto const& pieceEntry : b.pieceSquare)
        {
            Creature* candidate = botAI->GetCreature(pieceEntry.first);
            if (candidate && candidate->IsAlive() && IsKingChessPieceEntry(candidate->GetEntry()) &&
                IsEnemyChessPieceForBot(bot, candidate))
            {
                enemyKing = candidate;
                break;
            }
        }
        int forward = 0;
        if (enemyKing)
        {
            auto itEnemy = b.pieceSquare.find(enemyKing->GetGUID());
            if (itEnemy != b.pieceSquare.end())
                forward = (itEnemy->second.row > sq.row) ? 1 : -1;
        }
        if (forward == 0)
            forward = 1;

        std::set<std::pair<int, int>> candidates;
        uint32 e = piece->GetEntry();
        if (e == NPC_PAWN_A || e == NPC_PAWN_H)
        {
            AppendStepMoves(b, sq.row, sq.col, forward, 0, candidates);
        }
        else if (e == NPC_KNIGHT_A || e == NPC_KNIGHT_H)
        {
            int d[8][2] = { {2,1},{2,-1},{-2,1},{-2,-1},{1,2},{1,-2},{-1,2},{-1,-2} };
            for (auto& v : d)
                AppendStepMoves(b, sq.row, sq.col, v[0], v[1], candidates);
        }
        else if (e == NPC_BISHOP_A || e == NPC_BISHOP_H)
        {
            AppendRayMoves(b, sq.row, sq.col, 1, 1, candidates);
            AppendRayMoves(b, sq.row, sq.col, 1, -1, candidates);
            AppendRayMoves(b, sq.row, sq.col, -1, 1, candidates);
            AppendRayMoves(b, sq.row, sq.col, -1, -1, candidates);
        }
        else if (e == NPC_ROOK_A || e == NPC_ROOK_H)
        {
            AppendRayMoves(b, sq.row, sq.col, 1, 0, candidates);
            AppendRayMoves(b, sq.row, sq.col, -1, 0, candidates);
            AppendRayMoves(b, sq.row, sq.col, 0, 1, candidates);
            AppendRayMoves(b, sq.row, sq.col, 0, -1, candidates);
        }
        else if (e == NPC_QUEEN_A || e == NPC_QUEEN_H)
        {
            AppendRayMoves(b, sq.row, sq.col, 1, 1, candidates);
            AppendRayMoves(b, sq.row, sq.col, 1, -1, candidates);
            AppendRayMoves(b, sq.row, sq.col, -1, 1, candidates);
            AppendRayMoves(b, sq.row, sq.col, -1, -1, candidates);
            AppendRayMoves(b, sq.row, sq.col, 1, 0, candidates);
            AppendRayMoves(b, sq.row, sq.col, -1, 0, candidates);
            AppendRayMoves(b, sq.row, sq.col, 0, 1, candidates);
            AppendRayMoves(b, sq.row, sq.col, 0, -1, candidates);
        }
        else
        {
            for (int dr = -1; dr <= 1; ++dr)
                for (int dc = -1; dc <= 1; ++dc)
                    if (dr || dc)
                        AppendStepMoves(b, sq.row, sq.col, dr, dc, candidates);
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
            ClearPendingChessClaim(bot, "timeout");
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


                if (!pending->controlAttempted)
                {
                    bot->CastSpell(pendingPiece, SPELL_CONTROL_PIECE, true);
                    pending->controlAttempted = true;
                    pending->expiresAt = now + 5;
                }

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
        if (openingProgressConfirmed && !IsPawnEntry(assigned->GetEntry()))
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
        bot->CastSpell(assigned, SPELL_CONTROL_PIECE, true);
        if (ChessPendingClaim* pending = GetPendingChessClaim(bot))
        {
            if (pending->pieceGuid == assigned->GetGUID() && !pending->controlAttempted)
            {
                pending->controlAttempted = true;
                pending->expiresAt = now + 5;
            }
        }
        return true;
    }

    std::vector<Creature*> nearby = GetNearbyChessPieces(botAI, bot, false);
    if (nearby.empty())
    {
        return false;
    }

    Creature* best = nullptr;
    uint32 bestScore = 0;
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

        if (phase == ChessPhase::OPENING)
        {
            if (IsPawnEntry(piece->GetEntry()))
                score += 1200;
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

        if (!best || score > bestScore)
        {
            best = piece;
            bestScore = score;
        }
    }

    if (!best || !SetAssignedChessPiece(bot, best, 10))
    {
        return false;
    }

    if (openingProgressConfirmed && !IsPawnEntry(best->GetEntry()))
        SetPendingChessClaim(bot, best, instanceId, now, 30);

    float bestDist = bot->GetExactDist2d(best);
    if (bestDist > 8.0f)
    {
        return MoveTo(KARAZHAN_MAP_ID, best->GetPositionX(), best->GetPositionY(), best->GetPositionZ(),
                      false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    bot->CastSpell(best, SPELL_CONTROL_PIECE, true);
    if (ChessPendingClaim* pending = GetPendingChessClaim(bot))
    {
        if (pending->pieceGuid == best->GetGUID() && !pending->controlAttempted)
        {
            pending->controlAttempted = true;
            pending->expiresAt = now + 5;
        }
    }
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
    std::string const pieceSpellList = GetChessPieceSpellList(piece);
    std::vector<std::pair<float, Creature*>> legalMoves;
    ObjectGuid const pieceGuid = piece->GetGUID();
    auto moveIt = chessLastMoveCommandByPiece.find(pieceGuid);
    if (moveIt != chessLastMoveCommandByPiece.end() && (nowTick - moveIt->second) < 2)
    {
        return false;
    }

    ObjectGuid const lastFailed = chessLastFailedMoveTriggerByPiece[pieceGuid];
    bool const recentFailure = chessLastFailedMoveTimeByPiece.count(pieceGuid) && (nowTick - chessLastFailedMoveTimeByPiece[pieceGuid]) < 2;

    for (Creature* trigger : triggers)
    {
        float distPiece = piece->GetExactDist2d(trigger);
        float distKing = trigger->GetExactDist2d(enemyKing);
        if (distPiece > 30.0f)
            continue;

        // Avoid immediate retry on the same blocked square for one short tick window.
        if (recentFailure && trigger->GetGUID() == lastFailed)
            continue;

        legalMoves.push_back({ distKing, trigger });
    }

    if (legalMoves.empty())
    {
        int forward = 0;
        if (pieceSqIt != board.pieceSquare.end() && kingSqIt != board.pieceSquare.end())
            forward = (kingSqIt->second.row > pieceSqIt->second.row) ? 1 : -1;
        if (forward == 0)
            forward = 1;
        bool const hasMoveSpell = moveSpell != 0;

        std::string curSq = "unknown";
        if (pieceSqIt != board.pieceSquare.end())
            curSq = "(" + std::to_string(pieceSqIt->second.row) + "," + std::to_string(pieceSqIt->second.col) + ")";

        std::string squareBounds = "rows=" + std::to_string(board.minRow) + ".." + std::to_string(board.maxRow) +
                                   " cols=" + std::to_string(board.minCol) + ".." + std::to_string(board.maxCol);
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
        int bestForward = 0;
        float bestMoveScore = -std::numeric_limits<float>::max();
        Creature* bestForwardTrigger = nullptr;
        Creature* moveTarget = nullptr;
        std::string moveTargetReason = "enemy_king";
        float targetDistBefore = std::numeric_limits<float>::max();
        float targetDistAfter = std::numeric_limits<float>::max();
        bool const nonPawnMovement = controlledNonPawn;
        if (nonPawnMovement)
        {
            ChessOffensiveTargetSelection movementTarget =
                IsSummonedDaemonChessPiece(piece->GetEntry())
                    ? SelectSummonedDaemonChessTarget(botAI, bot, piece, board, "movement")
                    : SelectNonKingChessTarget(botAI, bot, piece, board, "movement");
            if (movementTarget.target)
            {
                moveTarget = movementTarget.target;
                moveTargetReason = "selected_nonking_" + movementTarget.category;
                targetDistBefore = movementTarget.distance;
            }
            else if (enemyKing)
            {
                uint32 supportAlive = 0;
                uint32 damageAlive = 0;
                uint32 pawnAlive = 0;
                std::string gateReason = "no_enemy_king";
                bool const kingMovementAllowed = IsKarazhanChessKingFocusAllowedActiveBoard(bot, board, enemyKing, supportAlive, damageAlive, pawnAlive, gateReason);
                bool const noActionableMovementTarget = IsNoActionableNonKingRejectReason(movementTarget.rejectReason);
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
                    moveTargetReason = movementDeadlockBreaker ? "enemy_king_fallback_breaker" : "enemy_king_fallback";
                    targetDistBefore = piece->GetExactDist2d(enemyKing);
                }
                else
                {
                    return false;
                }
            }

            if (IsSummonedDaemonChessPiece(piece->GetEntry()))
            {
                float const practicalAttackRange = GetChessPracticalAttackRange(piece);
                if (!moveTarget)
                {
                }
            }

            if (moveTarget)
            {
                float const practicalAttackRange = GetChessPracticalAttackRange(piece);
                if (IsSummonedDaemonChessPiece(piece->GetEntry()))
                {
                    constexpr float HellfireAoeRadius = 10.0f;
                    uint32 const nearbyEnemyCount = CountNearbyActiveBoardEnemyChessPieces(bot, board, piece, HellfireAoeRadius);
                    if (nearbyEnemyCount)
                    {
                        return false;
                    }
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

            // prevent direct reverse unless emergency
            auto lastFromIt = chessLastMoveFromSquareByPiece.find(piece->GetGUID());
            auto lastToIt = chessLastMoveToSquareByPiece.find(piece->GetGUID());
            if (lastFromIt != chessLastMoveFromSquareByPiece.end() &&
                lastToIt != chessLastMoveToSquareByPiece.end() &&
                lastToIt->second.row == curSq.row && lastToIt->second.col == curSq.col &&
                lastFromIt->second.row == row && lastFromIt->second.col == col)
                continue;

            int dRow = row - curSq.row;
            int absForward = std::abs(dRow);
            float candidateScore = static_cast<float>(absForward) * 15.0f;
            int edgePenalty = 0;
            int cornerPenalty = 0;
            int centerPenalty = 0;
            bool const onEdge = row == board.minRow || row == board.maxRow || col == board.minCol || col == board.maxCol;
            bool const onCorner = (row == board.minRow || row == board.maxRow) && (col == board.minCol || col == board.maxCol);
            if (onEdge)
                edgePenalty = 5;
            if (onCorner)
                cornerPenalty = 15;

            int const centerRow = (board.minRow + board.maxRow) / 2;
            int const centerCol = (board.minCol + board.maxCol) / 2;
            centerPenalty = std::abs(row - centerRow) + std::abs(col - centerCol);

            if (moveTarget)
            {
                targetDistAfter = moveTarget->GetExactDist2d(mv.second);
                candidateScore += (targetDistBefore - targetDistAfter) * 100.0f;
            }

            candidateScore -= static_cast<float>(edgePenalty + cornerPenalty + centerPenalty);


            if (!bestForwardTrigger || candidateScore > bestMoveScore ||
                (candidateScore == bestMoveScore && absForward > bestForward))
            {
                bestForward = absForward;
                bestMoveScore = candidateScore;
                bestForwardTrigger = mv.second;
            }
        }

        if (!bestForwardTrigger)
        {
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

        if (!moveSpell)
        {
            return false;
        }

        piece->CastSpell(bestForwardTrigger, moveSpell, true);
        RecordPendingChessMove(
            bot, piece, instanceId, curSq, ChessSquare{ toRow, toCol }, moveSpell,
            bestForwardTrigger->GetGUID(), "pawn-move", nowTick, 5);
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

    if (!moveSpell)
    {
        return false;
    }

    piece->CastSpell(safe, moveSpell, true);
    RecordPendingChessMove(
        bot, piece, instanceId, hasOldSq ? oldSq : ChessSquare{}, ChessSquare{ toRow, toCol }, moveSpell,
        safe->GetGUID(), "move-out-of-fire", now, 5);
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

    Creature* friendlyKing = GetFriendlyChessKing(botAI, bot);
    Creature* enemyKing = GetEnemyChessKing(botAI, bot);
    const bool canMove = !piece->HasAura(SPELL_MOVE_COOLDOWN);
    const bool needsMovement = canMove && enemyKing && piece->GetExactDist2d(enemyKing) > 22.0f;
    const bool healerNeedsTarget = IsHealerChessPieceEntry(piece->GetEntry()) && friendlyKing && friendlyKing->GetHealthPct() < 90.0f;
    const time_t now = std::time(nullptr);
    const ObjectGuid pieceGuid = piece->GetGUID();
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
        nonKingTarget =
            IsSummonedDaemonChessPiece(piece->GetEntry())
                ? SelectSummonedDaemonChessTarget(botAI, bot, piece, board, "offense")
                : SelectNonKingChessTarget(botAI, bot, piece, board, "offense");

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
                bool const supportAllowed = IsChessSupportSpellForNonKingTarget(candidateSpellId, supportRejectReason);
            }
        }
        if (spellCandidates.empty())
        {
            supportTarget = SelectDamagedFriendlyActiveBoardNonKingTarget(bot, board, piece);

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
                bool const supportAllowed = IsChessSupportSpellForNonKingTarget(supportSpellId, supportRejectReason);
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
                if (piece->GetEntry() == NPC_BISHOP_H || piece->GetEntry() == NPC_BISHOP_A)
                {
                }
                piece->CastSpell(friendlyKing, spellId, true);
                if (piece->HasSpellCooldown(spellId))
                {
                    ClearChessSpellNoOpBackoff(pieceGuid, spellId);
                    StampChessPieceCommandGcd(piece);
                    chessLastAbilityCommandByPiece[pieceGuid] = now;
                    if (openingPawnMoved)
                    return true;
                }
                StampChessSpellNoOpBackoff(pieceGuid, spellId, now, 2);
                continue;
            }

            if ((piece->GetEntry() == NPC_BISHOP_H || piece->GetEntry() == NPC_BISHOP_A) && spellId == 37456)
            {
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
                    !IsKingChessPieceEntry(supportTarget.target->GetEntry()) &&
                    supportSqIt != board.pieceSquare.end() &&
                    board.occupied.find({ supportSqIt->second.row, supportSqIt->second.col }) != board.occupied.end();
                if (!supportTargetValid)
                {
                    continue;
                }

                piece->CastSpell(supportTarget.target, spellId, true);
                if (piece->HasSpellCooldown(spellId))
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
            if (IsChessHealSpellBlockedOnEnemy(spellId))
                continue;
            piece->CastSpell(enemyKing, spellId, true);
            if (piece->HasSpellCooldown(spellId))
            {
                ClearChessSpellNoOpBackoff(pieceGuid, spellId);
                StampChessPieceCommandGcd(piece);
                chessLastAbilityCommandByPiece[pieceGuid] = now;
                if (IsPoisonCloudChessSpell(spellId))
                    chessPoisonCloudLastAppliedByTarget[enemyKing->GetGUID()] = now;
                return true;
            }
            StampChessSpellNoOpBackoff(pieceGuid, spellId, now, 2);
            continue;
        }
        if (useNonKingOffense && nonKingTarget.target && offensiveKingSpell && !effectiveKingAttackAllowed)
        {
            if (phase == ChessPhase::OPENING && !openingPawnMoved)
            {
                continue;
            }

            if (IsHellfireChessAoeSpell(spellId))
            {
                // Conservative chess-safe radius: Hellfire should only be used if enemies are actually nearby.
                constexpr float HellfireAoeRadius = 10.0f;
                uint32 nearbyEnemyCount = CountNearbyActiveBoardEnemyChessPieces(bot, board, piece, HellfireAoeRadius);
                float const kingDistance = enemyKing ? piece->GetExactDist2d(enemyKing) : 0.0f;
                if (!nearbyEnemyCount)
                {
                    continue;
                }
                if (IsSummonedDaemonChessPiece(piece->GetEntry()) && enemyKing && enemyDamageAlive <= 1)
                {
                    constexpr float HellfireSafetyMargin = 0.5f;
                    if (kingDistance > (HellfireAoeRadius - HellfireSafetyMargin))
                    {
                        return false;
                    }
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
            piece->CastSpell(nonKingTarget.target, spellId, true);
            if (piece->HasSpellCooldown(spellId))
            {
                ClearChessSpellNoOpBackoff(pieceGuid, spellId);
                StampChessPieceCommandGcd(piece);
                chessLastAbilityCommandByPiece[pieceGuid] = now;
                if (IsPoisonCloudChessSpell(spellId))
                    chessPoisonCloudLastAppliedByTarget[nonKingTarget.target->GetGUID()] = now;
                return true;
            }

            StampChessSpellNoOpBackoff(pieceGuid, spellId, now, 2);
            continue;
        }
        if (enemyKing && offensiveKingSpell && !effectiveKingAttackAllowed)
        {
            if (useNonKingOffense)
            {
            }

            if (IsHellfireChessAoeSpell(spellId))
            {
                // Conservative chess-safe radius: Hellfire should only be used if enemies are actually nearby.
                constexpr float HellfireAoeRadius = 10.0f;
                uint32 nearbyEnemyCount = CountNearbyActiveBoardEnemyChessPieces(bot, board, piece, HellfireAoeRadius);
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

        piece->CastSpell(piece, spellId, true);
        if (piece->HasSpellCooldown(spellId))
        {
            selfThrottle = now;
            StampChessPieceCommandGcd(piece);
            chessLastAbilityCommandByPiece[pieceGuid] = now;
            ClearChessSpellNoOpBackoff(pieceGuid, spellId);
            return true;
        }
        StampChessSpellNoOpBackoff(pieceGuid, spellId, now, 2);
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
    Creature* king = GetFriendlyChessKing(botAI, bot);
    if (!king)
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
    const time_t now = std::time(nullptr);
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
            bool const offensiveAllowed = IsKingAttackOffensiveChessSpell(spellId, rejectReason);
            if (!offensiveAllowed)
            {
                continue;
            }

            time_t backoffRemaining = 0;
            if (IsChessSpellNoOpBackoffActive(piece->GetGUID(), spellId, now, backoffRemaining))
            {
                continue;
            }

            piece->CastSpell(king, spellId, true);
            if (piece->HasSpellCooldown(spellId))
            {
                ClearChessSpellNoOpBackoff(piece->GetGUID(), spellId);
                StampChessPieceCommandGcd(piece);
                chessLastAbilityCommandByPiece[piece->GetGUID()] = now;
            }
            else
            {
                StampChessSpellNoOpBackoff(piece->GetGUID(), spellId, now, 2);
            }
            return true;
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
        IsSummonedDaemonChessPiece(piece->GetEntry())
            ? SelectSummonedDaemonChessTarget(botAI, bot, piece, board, "offense")
            : SelectNonKingChessTarget(botAI, bot, piece, board, "offense");
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

            piece->CastSpell(enemyKing, spellId, true);
            if (piece->HasSpellCooldown(spellId))
            {
                ClearChessSpellNoOpBackoff(piece->GetGUID(), spellId);
                StampChessPieceCommandGcd(piece);
                chessLastAbilityCommandByPiece[piece->GetGUID()] = now;
                return true;
            }

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
        float d = trigger->GetExactDist2d(midX, midY);
        if (d < bestDist)
        {
            bestDist = d;
            best = trigger;
        }
    }

    if (!best)
        return false;

    if (!moveSpell)
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

    piece->CastSpell(best, moveSpell, true);
    RecordPendingChessMove(
        bot, piece, instanceId, oldSq, ChessSquare{ toRow, toCol }, moveSpell, best->GetGUID(), "block-path", now, 5);
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
            chessEventStartByInstance.erase(bot->GetMap()->GetInstanceId());
        if (Creature* controlled = bot->GetCharm() ? bot->GetCharm()->ToCreature() : nullptr)
        {
            ObjectGuid pg = controlled->GetGUID();
            chessPendingMoveByPiece.erase(pg);
            chessLastSquareByPiece.erase(pg);
            chessLastMoveFromSquareByPiece.erase(pg);
            chessLastMoveToSquareByPiece.erase(pg);
            chessLastMoveCommandByPiece.erase(pg);
            chessLastAbilityCommandByPiece.erase(pg);
            chessMovementCooldownUntilByPiece.erase(pg);
            chessAbilityNoOpBackoffByPiece.erase(pg);
            chessPawnOpeningAdvanceCountByPiece.erase(pg);
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
        chessAbilityNoOpBackoffByPiece.erase(charm->GetGUID());
        chessPawnOpeningAdvanceCountByPiece.erase(charm->GetGUID());
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
        chessAbilityNoOpBackoffByPiece.erase(assigned->GetGUID());
        chessPawnOpeningAdvanceCountByPiece.erase(assigned->GetGUID());
        ClearPendingChessClaim(bot, "assigned-invalid");
        ClearAssignedChessPiece(bot);
    }
    else if (assigned)
    {
        ChessBoardState board = BuildChessBoardState(botAI, bot);
        ChessPhase phase = GetChessPhase(botAI, bot, board);
        bool const openingProgressConfirmed = HasConfirmedOpeningProgress(bot->GetMap() ? bot->GetMap()->GetInstanceId() : 0);
        bool const pawnOnlyMode = !openingProgressConfirmed;
        bool const isPawn = IsPawnEntry(assigned->GetEntry());
        bool const isFriendlyForBot = IsFriendlyChessPieceForBot(bot, assigned);
        bool const notSelectable = assigned->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        bool const assignedToOtherBot = IsPieceAssignedToOtherBot(bot, assigned);

        bool const pawnOpenedLane = openingProgressConfirmed && isPawn && HasPawnMovedDuringOpening(bot->GetMap() ? bot->GetMap()->GetInstanceId() : 0, assigned->GetGUID());
        bool const pawnEnemyContact = openingProgressConfirmed && isPawn && (assigned->IsInCombat() || assigned->GetVictim());
        ChessSquare assignedSq{};
        bool const hasAssignedSq = board.pieceSquare.find(assigned->GetGUID()) != board.pieceSquare.end();
        if (hasAssignedSq)
            assignedSq = board.pieceSquare.find(assigned->GetGUID())->second;

        Creature* enemyKingForRetire = GetEnemyChessKing(botAI, bot);
        int forward = 0;
        if (hasAssignedSq && enemyKingForRetire)
        {
            auto kingSqIt = board.pieceSquare.find(enemyKingForRetire->GetGUID());
            if (kingSqIt != board.pieceSquare.end())
                forward = (kingSqIt->second.row > assignedSq.row) ? 1 : -1;
        }
        if (forward == 0)
            forward = 1;

        uint8 const advanceCount = GetPawnOpeningAdvanceCount(assigned->GetGUID());
        bool const centralPawn = hasAssignedSq ? IsCentralChessPawnFile(assignedSq) : false;
        bool const backlineRouteOpen = hasAssignedSq ? HasClearBacklineRoute(board, assignedSq, forward) : false;
        bool const laneDepthSatisfied = pawnEnemyContact ||
            (centralPawn ? advanceCount >= 2 : (advanceCount >= 1 && backlineRouteOpen));
        bool const pawnRetireCandidate = openingProgressConfirmed && isPawn && laneDepthSatisfied;

        if (openingProgressConfirmed && isPawn)
        {
        }

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
            chessAbilityNoOpBackoffByPiece.erase(assigned->GetGUID());
            chessPawnOpeningAdvanceCountByPiece.erase(assigned->GetGUID());
            chessLastAbilityCommandByPiece.erase(assigned->GetGUID());
            chessLastMoveCommandByPiece.erase(assigned->GetGUID());
            chessMovementCooldownUntilByPiece.erase(assigned->GetGUID());
            chessLastFailedMoveTriggerByPiece.erase(assigned->GetGUID());
            chessLastFailedMoveTimeByPiece.erase(assigned->GetGUID());
            chessOpeningMoveRetryUntilByPiece.erase(assigned->GetGUID());
            ClearPendingChessClaim(bot, "pawn-retired-for-upgrade");
            ClearAssignedChessPiece(bot);
            return false;
        }
        if (openingProgressConfirmed && isPawn)
        {
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
            chessAbilityNoOpBackoffByPiece.erase(assigned->GetGUID());
            ClearPendingChessClaim(bot, "assigned-not-allowed");
            ClearAssignedChessPiece(bot);
        }
    }

    return false;
}

// Netherspite

// One tank bot per phase will dance in and out of the red beam (5 seconds in, 5 seconds out)
// Tank bots will ignore void zones--their positioning is too important to risk losing beam control
bool NetherspiteBlockRedBeamAction::Execute(Event /*event*/)
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!netherspite)
        return false;

    Unit* redPortal = bot->FindNearestCreature(NPC_RED_PORTAL, 150.0f);
    if (!redPortal)
        return false;

    const ObjectGuid botGuid = bot->GetGUID();
    auto [redBlocker, greenBlocker, blueBlocker] = GetCurrentBeamBlockers(botAI, bot);
    bool isBlockingNow = (bot == redBlocker);

    auto it = _wasBlockingRedBeam.find(botGuid);
    bool wasBlocking = (it != _wasBlockingRedBeam.end()) ? it->second : false;

    Position beamPos = GetPositionOnBeam(netherspite, redPortal, 18.0f);

    if (isBlockingNow)
    {
        if (!wasBlocking)
        {
            std::map<std::string, std::string> placeholders{{"%player", bot->GetName()}};
            std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "netherspite_beam_blocking_red", "%player is moving to block the red beam!", placeholders);
            bot->Yell(text, LANG_UNIVERSAL);
        }
        _wasBlockingRedBeam[botGuid] = true;

        const uint8 intervalSecs = 5;
        if (std::time(nullptr) - redBeamMoveTimer[botGuid] >= intervalSecs)
        {
            lastBeamMoveSideways[botGuid] = !lastBeamMoveSideways[botGuid];
            redBeamMoveTimer[botGuid] = std::time(nullptr);
        }
        if (!lastBeamMoveSideways[botGuid])
            return MoveTo(KARAZHAN_MAP_ID, beamPos.GetPositionX(), beamPos.GetPositionY(), beamPos.GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
        else
        {
            float length = netherspite->GetExactDist2d(redPortal);
            if (length == 0.0f)
                return false;

            float dx = (redPortal->GetPositionX() - netherspite->GetPositionX()) / length;
            float dy = (redPortal->GetPositionY() - netherspite->GetPositionY()) / length;
            float perpDx = -dy;
            float perpDy = dx;
            float sideX = beamPos.GetPositionX() + perpDx * 3.0f;
            float sideY = beamPos.GetPositionY() + perpDy * 3.0f;
            float sideZ = beamPos.GetPositionZ();

            return MoveTo(KARAZHAN_MAP_ID, sideX, sideY, sideZ, false, false, false, false,
                          MovementPriority::MOVEMENT_FORCED, true, false);
        }
    }

    _wasBlockingRedBeam[botGuid] = false;
    return false;
}

Position NetherspiteBlockRedBeamAction::GetPositionOnBeam(Unit* netherspite, Unit* portal, float distanceFromBoss)
{
    float bx = netherspite->GetPositionX();
    float by = netherspite->GetPositionY();
    float bz = netherspite->GetPositionZ();
    float px = portal->GetPositionX();
    float py = portal->GetPositionY();

    float dx = px - bx;
    float dy = py - by;
    float length = netherspite->GetExactDist2d(px, py);
    if (length == 0.0f)
        return Position(bx, by, bz);

    dx /= length;
    dy /= length;
    float targetX = bx + dx * distanceFromBoss;
    float targetY = by + dy * distanceFromBoss;
    float targetZ = bz;

    return Position(targetX, targetY, targetZ);
}

// Two non-Rogue/Warrior DPS bots will block the blue beam for each phase (swap at 25 debuff stacks)
// When avoiding void zones, blocking bots will move along the beam to continue blocking
bool NetherspiteBlockBlueBeamAction::Execute(Event /*event*/)
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!netherspite)
        return false;

    Unit* bluePortal = bot->FindNearestCreature(NPC_BLUE_PORTAL, 150.0f);
    if (!bluePortal)
        return false;

    const ObjectGuid botGuid = bot->GetGUID();
    auto [redBlocker, greenBlocker, blueBlocker] = GetCurrentBeamBlockers(botAI, bot);
    bool isBlockingNow = (bot == blueBlocker);

    auto it = _wasBlockingBlueBeam.find(botGuid);
    bool wasBlocking = (it != _wasBlockingBlueBeam.end()) ? it->second : false;

    if (wasBlocking && !isBlockingNow)
    {
        std::map<std::string, std::string> placeholders{{"%player", bot->GetName()}};
        std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "netherspite_beam_leaving_blue", "%player is leaving the blue beam--next blocker up!", placeholders);
        bot->Yell(text, LANG_UNIVERSAL);
        _wasBlockingBlueBeam[botGuid] = false;
    }

    if (isBlockingNow)
    {
        if (!wasBlocking)
        {
            std::map<std::string, std::string> placeholders{{"%player", bot->GetName()}};
            std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "netherspite_beam_blocking_blue", "%player is moving to block the blue beam!", placeholders);
            bot->Yell(text, LANG_UNIVERSAL);
        }
        _wasBlockingBlueBeam[botGuid] = true;

        float idealDistance = botAI->IsRanged(bot) ? 25.0f : 18.0f;
        std::vector<Unit*> voidZones = GetAllVoidZones(botAI, bot);

        float bx = netherspite->GetPositionX();
        float by = netherspite->GetPositionY();
        float bz = netherspite->GetPositionZ();
        float px = bluePortal->GetPositionX();
        float py = bluePortal->GetPositionY();

        float dx = px - bx;
        float dy = py - by;
        float length = netherspite->GetExactDist2d(bluePortal);
        if (length == 0.0f)
            return false;

        dx /= length;
        dy /= length;
        float bestDist = 150.0f;
        Position bestPos;
        bool found = false;

        for (float dist = 18.0f; dist <= 30.0f; dist += 0.5f)
        {
            float candidateX = bx + dx * dist;
            float candidateY = by + dy * dist;
            float candidateZ = bz;
            if (!IsSafePosition(candidateX, candidateY, voidZones, 4.0f))
                continue;

            float distToIdeal = fabs(dist - idealDistance);
            if (!found || distToIdeal < bestDist)
            {
                bestDist = distToIdeal;
                bestPos = Position(candidateX, candidateY, candidateZ);
                found = true;
            }
        }

        if (found)
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
            return MoveTo(KARAZHAN_MAP_ID, bestPos.GetPositionX(), bestPos.GetPositionY(), bestPos.GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
        }

        return false;
    }

    _wasBlockingBlueBeam[botGuid] = false;
    return false;
}

// Two healer bots will block the green beam for each phase (swap at 25 debuff stacks)
// OR one rogue or DPS warrior bot will block the green beam for an entire phase (if they begin the phase as the blocker)
// When avoiding void zones, blocking bots will move along the beam to continue blocking
bool NetherspiteBlockGreenBeamAction::Execute(Event /*event*/)
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!netherspite)
        return false;

    Unit* greenPortal = bot->FindNearestCreature(NPC_GREEN_PORTAL, 150.0f);
    if (!greenPortal)
        return false;

    const ObjectGuid botGuid = bot->GetGUID();
    auto [redBlocker, greenBlocker, blueBlocker] = GetCurrentBeamBlockers(botAI, bot);
    bool isBlockingNow = (bot == greenBlocker);

    auto it = _wasBlockingGreenBeam.find(botGuid);
    bool wasBlocking = (it != _wasBlockingGreenBeam.end()) ? it->second : false;

    if (wasBlocking && !isBlockingNow)
    {
        std::map<std::string, std::string> placeholders{{"%player", bot->GetName()}};
        std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "netherspite_beam_leaving_green", "%player is leaving the green beam--next blocker up!", placeholders);
        bot->Yell(text, LANG_UNIVERSAL);
        _wasBlockingGreenBeam[botGuid] = false;
    }

    if (isBlockingNow)
    {
        if (!wasBlocking)
        {
            std::map<std::string, std::string> placeholders{{"%player", bot->GetName()}};
            std::string text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
                "netherspite_beam_blocking_green", "%player is moving to block the green beam!", placeholders);
            bot->Yell(text, LANG_UNIVERSAL);
        }
        _wasBlockingGreenBeam[botGuid] = true;

        std::vector<Unit*> voidZones = GetAllVoidZones(botAI, bot);

        float bx = netherspite->GetPositionX();
        float by = netherspite->GetPositionY();
        float bz = netherspite->GetPositionZ();
        float px = greenPortal->GetPositionX();
        float py = greenPortal->GetPositionY();

        float dx = px - bx;
        float dy = py - by;
        float length = netherspite->GetExactDist2d(greenPortal);
        if (length == 0.0f)
            return false;

        dx /= length;
        dy /= length;
        float bestDist = 150.0f;
        Position bestPos;
        bool found = false;

        for (float dist = 18.0f; dist <= 30.0f; dist += 0.5f)
        {
            float candidateX = bx + dx * dist;
            float candidateY = by + dy * dist;
            float candidateZ = bz;
            if (!IsSafePosition(candidateX, candidateY, voidZones, 4.0f))
                continue;

            float distToIdeal = fabs(dist - 18.0f);
            if (!found || distToIdeal < bestDist)
            {
                bestDist = distToIdeal;
                bestPos = Position(candidateX, candidateY, candidateZ);
                found = true;
            }
        }

        if (found)
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
            return MoveTo(KARAZHAN_MAP_ID, bestPos.GetPositionX(), bestPos.GetPositionY(), bestPos.GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
        }

        return false;
    }

    _wasBlockingGreenBeam[botGuid] = false;
    return false;
}

// All bots not currently blocking a beam will avoid beams and void zones
bool NetherspiteAvoidBeamAndVoidZoneAction::Execute(Event /*event*/)
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!netherspite)
        return false;

    std::vector<Unit*> voidZones = GetAllVoidZones(botAI, bot);

    bool nearVoidZone = !IsSafePosition(bot->GetPositionX(), bot->GetPositionY(),
                                        voidZones, 4.0f);

    std::vector<BeamAvoid> beams;
    Unit* redPortal = bot->FindNearestCreature(NPC_RED_PORTAL, 150.0f);
    Unit* bluePortal = bot->FindNearestCreature(NPC_BLUE_PORTAL, 150.0f);
    Unit* greenPortal = bot->FindNearestCreature(NPC_GREEN_PORTAL, 150.0f);

    if (redPortal)
    {
        float length = netherspite->GetExactDist2d(redPortal);
        beams.push_back({redPortal, 0.0f, length});
    }

    if (bluePortal)
    {
        float length = netherspite->GetExactDist2d(bluePortal);
        beams.push_back({bluePortal, 0.0f, length});
    }

    if (greenPortal)
    {
        float length = netherspite->GetExactDist2d(greenPortal);
        beams.push_back({greenPortal, 0.0f, length});
    }

    bool nearBeam = !IsAwayFromBeams(bot->GetPositionX(), bot->GetPositionY(), beams, netherspite);

    if (!nearVoidZone && !nearBeam)
        return false;

    const float minMoveDist = 2.0f;
    const float minMoveDistSq = minMoveDist * minMoveDist;
    const float maxSearchDist = 30.0f, stepAngle = M_PI/18.0f, stepDist = 0.5f;
    float netherspiteZ = netherspite->GetPositionZ();
    Position bestCandidate;
    float bestDistSq = std::numeric_limits<float>::max();
    bool found = false;

    const float botX = bot->GetPositionX();
    const float botY = bot->GetPositionY();

    for (float angle = 0; angle < 2 * M_PI; angle += stepAngle)
    {
        for (float dist = 2.0f; dist <= maxSearchDist; dist += stepDist)
        {
            float cx = botX + std::cos(angle) * dist;
            float cy = botY + std::sin(angle) * dist;
            float cz = netherspiteZ;

            if (!IsSafePosition(cx, cy, voidZones, 4.0f) ||
                !IsAwayFromBeams(cx, cy, beams, netherspite))
                continue;

            float dx = cx - botX;
            float dy = cy - botY;
            float moveDistSq = dx*dx + dy*dy;
            if (moveDistSq < minMoveDistSq)
                continue;

            if (!found || moveDistSq < bestDistSq)
            {
                bestCandidate = Position(cx, cy, cz);
                bestDistSq = moveDistSq;
                found = true;
            }
        }
    }

    if (found)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return MoveTo(KARAZHAN_MAP_ID, bestCandidate.GetPositionX(), bestCandidate.GetPositionY(),
                      bestCandidate.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

bool NetherspiteAvoidBeamAndVoidZoneAction::IsAwayFromBeams(
     float x, float y, const std::vector<BeamAvoid>& beams, Unit* netherspite)
{
    for (auto const& beam : beams)
    {
        float bx = netherspite->GetPositionX(), by = netherspite->GetPositionY();
        float px = beam.portal->GetPositionX(), py = beam.portal->GetPositionY();
        float dx = px - bx, dy = py - by;
        float length = netherspite->GetExactDist2d(beam.portal);

        if (length == 0.0f)
            continue;

        dx /= length; dy /= length;
        float botdx = x - bx, botdy = y - by;
        float distanceAlongBeam = (botdx * dx + botdy * dy);
        float beamX = bx + dx * distanceAlongBeam, beamY = by + dy * distanceAlongBeam;
        float distToBeam = sqrt((x - beamX) * (x - beamX) + (y - beamY) * (y - beamY));

        if (distToBeam < 5.0f && distanceAlongBeam > beam.minDist && distanceAlongBeam < beam.maxDist)
            return false;
    }

    return true;
}

bool NetherspiteBanishPhaseAvoidVoidZoneAction::Execute(Event /*event*/)
{
    std::vector<Unit*> voidZones = GetAllVoidZones(botAI, bot);

    for (Unit* vz : voidZones)
    {
        if (vz->GetEntry() == NPC_VOID_ZONE && bot->GetExactDist2d(vz) < 4.0f)
            return FleePosition(vz->GetPosition(), 4.0f);
    }

    return false;
}

bool NetherspiteManageTimersAndTrackersAction::Execute(Event /*event*/)
{
    Unit* netherspite = AI_VALUE2(Unit*, "find target", "netherspite");
    if (!netherspite)
        return false;

    const uint32 instanceId = netherspite->GetMap()->GetInstanceId();
    const ObjectGuid botGuid = bot->GetGUID();
    const time_t now = std::time(nullptr);

    // DpsWaitTimer is for pausing DPS during phase transitions
    // redBeamMoveTimer and lastBeamMoveSideways are for tank dancing in/out of the red beam
    if (netherspite->GetHealth() == netherspite->GetMaxHealth() &&
        !netherspite->HasAura(SPELL_GREEN_BEAM_HEAL))
    {
        if (IsMechanicTrackerBot(botAI, bot, KARAZHAN_MAP_ID, nullptr))
            netherspiteDpsWaitTimer.insert_or_assign(instanceId, now);

        if (botAI->IsTank(bot) && !bot->HasAura(SPELL_RED_BEAM_DEBUFF))
        {
            redBeamMoveTimer.erase(botGuid);
            lastBeamMoveSideways.erase(botGuid);
        }
    }
    else if (netherspite->HasAura(SPELL_NETHERSPITE_BANISHED))
    {
        if (IsMechanicTrackerBot(botAI, bot, KARAZHAN_MAP_ID, nullptr))
            netherspiteDpsWaitTimer.erase(instanceId);

        if (botAI->IsTank(bot))
        {
            redBeamMoveTimer.erase(botGuid);
            lastBeamMoveSideways.erase(botGuid);
        }
    }
    else if (!netherspite->HasAura(SPELL_NETHERSPITE_BANISHED))
    {
        if (IsMechanicTrackerBot(botAI, bot, KARAZHAN_MAP_ID, nullptr))
            netherspiteDpsWaitTimer.try_emplace(instanceId, now);

        if (botAI->IsTank(bot) && bot->HasAura(SPELL_RED_BEAM_DEBUFF))
        {
            redBeamMoveTimer.try_emplace(botGuid, now);
            lastBeamMoveSideways.try_emplace(botGuid, false);
        }
    }

    return false;
}

// Move away from the boss to avoid Shadow Nova when Enfeebled
// Do not cross within Infernal Hellfire radius while doing so
bool PrinceMalchezaarEnfeebledAvoidHazardAction::Execute(Event /*event*/)
{
    Unit* malchezaar = AI_VALUE2(Unit*, "find target", "prince malchezaar");
    if (!malchezaar)
        return false;

    std::vector<Unit*> infernals = GetSpawnedInfernals(botAI);

    const float minSafeBossDistance = 34.0f;
    const float minSafeBossDistanceSq = minSafeBossDistance * minSafeBossDistance;
    const float maxSafeBossDistance = 60.0f;
    const float safeInfernalDistance = 23.0f;
    const float distIncrement = 0.5f;
    const uint8 numAngles = 64;

    float bx = bot->GetPositionX();
    float by = bot->GetPositionY();
    float bz = bot->GetPositionZ();
    float malchezaarX = malchezaar->GetPositionX();
    float malchezaarY = malchezaar->GetPositionY();
    float malchezaarZ = malchezaar->GetPositionZ();
    float bestMoveDistSq = std::numeric_limits<float>::max();
    float bestDestX = 0.0f, bestDestY = 0.0f, bestDestZ = bz;
    bool found = false;

    for (int i = 0; i < numAngles; ++i)
    {
        float angle = (2 * M_PI * i) / numAngles;
        float dx = std::cos(angle);
        float dy = std::sin(angle);

        for (float dist = minSafeBossDistance; dist <= maxSafeBossDistance; dist += distIncrement)
        {
            float x = malchezaarX + dx * dist;
            float y = malchezaarY + dy * dist;
            float destZ = malchezaarZ;
            float destX = x, destY = y;
            if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bx, by, bz, destX, destY, destZ, true))
                continue;

            float ddx = destX - malchezaarX;
            float ddy = destY - malchezaarY;
            float distFromBossSq = ddx*ddx + ddy*ddy;
            if (distFromBossSq < minSafeBossDistanceSq)
                continue;

            bool pathSafe = IsStraightPathSafe(Position(bx, by, bz), Position(destX, destY, destZ),
                                               infernals, safeInfernalDistance, distIncrement);
            float mdx = destX - bx;
            float mdy = destY - by;
            float moveDistSq = mdx*mdx + mdy*mdy;

            if (pathSafe && moveDistSq < bestMoveDistSq)
            {
                bestMoveDistSq = moveDistSq;
                bestDestX = destX;
                bestDestY = destY;
                bestDestZ = destZ;
                found = true;
            }
        }
    }

    if (found)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return MoveTo(KARAZHAN_MAP_ID, bestDestX, bestDestY, bestDestZ, false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

// Move away from infernals while staying within range of the boss
// Prioritize finding a safe path to the new location, but will fallback to just finding a safe location if needed
bool PrinceMalchezaarNonTankAvoidInfernalAction::Execute(Event /*event*/)
{
    Unit* malchezaar = AI_VALUE2(Unit*, "find target", "prince malchezaar");
    if (!malchezaar)
        return false;

    std::vector<Unit*> infernals = GetSpawnedInfernals(botAI);

    const float safeInfernalDistance = 23.0f;
    const float safeInfernalDistanceSq = safeInfernalDistance * safeInfernalDistance;
    const float maxSafeBossDistance = 35.0f;

    float bx = bot->GetPositionX();
    float by = bot->GetPositionY();
    float bz = bot->GetPositionZ();
    float malchezaarX = malchezaar->GetPositionX();
    float malchezaarY = malchezaar->GetPositionY();
    float malchezaarZ = malchezaar->GetPositionZ();

    bool nearInfernal = false;
    for (Unit* infernal : infernals)
    {
        float dx = bx - infernal->GetPositionX();
        float dy = by - infernal->GetPositionY();
        float infernalDistSq = dx*dx + dy*dy;
        if (infernalDistSq < safeInfernalDistanceSq)
        {
            nearInfernal = true;
            break;
        }
    }

    float bestDestX = bx, bestDestY = by, bestDestZ = bz;
    bool found = false;

    if (nearInfernal)
    {
        const float distIncrement = 0.5f;
        const uint8 numAngles = 64;

        // 1. Try to find a safe position with a safe path
        found = TryFindSafePositionWithSafePath(bot, bx, by, bz, malchezaarX, malchezaarY, malchezaarZ,
                infernals, safeInfernalDistance, distIncrement, numAngles, maxSafeBossDistance,
                true, bestDestX, bestDestY, bestDestZ);

        // 2. Fallback: try to find a safe position (ignore path safety)
        if (!found)
        {
            found = TryFindSafePositionWithSafePath(bot, bx, by, bz, malchezaarX, malchezaarY, malchezaarZ,
                    infernals, safeInfernalDistance, distIncrement, numAngles, maxSafeBossDistance,
                    false, bestDestX, bestDestY, bestDestZ);
        }

        if (found)
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
            return MoveTo(KARAZHAN_MAP_ID, bestDestX, bestDestY, bestDestZ, false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    return false;
}

// This is similar to the non-tank avoid infernal action, but the movement is based on the bot's location
// And the safe distance from infernals is larger to give melee more room to maneuver
bool PrinceMalchezaarMainTankMovementAction::Execute(Event /*event*/)
{
    Unit* malchezaar = AI_VALUE2(Unit*, "find target", "prince malchezaar");
    if (!malchezaar)
        return false;

    if (AI_VALUE(Unit*, "current target") != malchezaar)
        return Attack(malchezaar);

    std::vector<Unit*> infernals = GetSpawnedInfernals(botAI);

    const float safeInfernalDistance = 30.0f;
    const float safeInfernalDistanceSq = safeInfernalDistance * safeInfernalDistance;
    const float maxSampleDist = 75.0f;

    float bx = bot->GetPositionX();
    float by = bot->GetPositionY();
    float bz = bot->GetPositionZ();

    bool nearInfernal = false;
    for (Unit* infernal : infernals)
    {
        float dx = bx - infernal->GetPositionX();
        float dy = by - infernal->GetPositionY();
        float infernalDistSq = dx*dx + dy*dy;
        if (infernalDistSq < safeInfernalDistanceSq)
        {
            nearInfernal = true;
            break;
        }
    }

    float bestDestX = bx, bestDestY = by, bestDestZ = bz;
    bool found = false;

    if (nearInfernal)
    {
        const float distIncrement = 0.5f;
        const uint8 numAngles = 64;

        // 1. Try to find a safe position with a safe path
        found = TryFindSafePositionWithSafePath( bot, bx, by, bz, bx, by, bz,
                infernals, safeInfernalDistance, distIncrement, numAngles, maxSampleDist,
                true, bestDestX, bestDestY, bestDestZ);

        // 2. Fallback: try to find a safe position (ignore path safety)
        if (!found)
        {
            found = TryFindSafePositionWithSafePath( bot, bx, by, bz, bx, by, bz,
                    infernals, safeInfernalDistance, distIncrement, numAngles, maxSampleDist,
                    false, bestDestX, bestDestY, bestDestZ);
        }

        if (found)
        {
            bot->AttackStop();
            return MoveTo(KARAZHAN_MAP_ID, bestDestX, bestDestY, bestDestZ, false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    return false;
}

// The tank position is near the Southeastern area of the Master's Terrace
// The tank moves Nightbane into position in two steps to try to get Nightbane to face sideways to the raid
bool NightbaneGroundPhasePositionBossAction::Execute(Event /*event*/)
{
    if (!IsInsideNightbaneFightArea(Position(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ())))
        return false;

    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (!nightbane)
    {
        return false;
    }
    if (ShouldUseDynamicHumanTankMode(botAI, bot, nightbane))
    {
        return false;
    }

    MarkTargetWithSkull(bot, nightbane);
    SetRtiTarget(botAI, "skull", nightbane);

    if (AI_VALUE(Unit*, "current target") != nightbane)
        return Attack(nightbane);

    const ObjectGuid botGuid = bot->GetGUID();
    uint8 step = nightbaneTankStep.count(botGuid) ? nightbaneTankStep[botGuid] : 0;

    if (nightbane->GetVictim() == bot)
    {
        const Position tankPositions[2] =
        {
            NIGHTBANE_TRANSITION_BOSS_POSITION,
            NIGHTBANE_FINAL_BOSS_POSITION
        };
        const Position& position = tankPositions[step];
        const float maxDistance = 0.5f;
        float distanceToTarget = bot->GetExactDist2d(position);

        if (distanceToTarget > maxDistance)
        {
            if (!IsNightbaneMovementAllowed(bot, position))
            {
                return false;
            }
            return MoveTo(KARAZHAN_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, true);
        }

        if (step == 0 && distanceToTarget <= maxDistance)
            nightbaneTankStep[botGuid] = 1;

        if (step == 1 && distanceToTarget <= maxDistance)
        {
            float orientation = atan2(nightbane->GetPositionY() - bot->GetPositionY(),
                                      nightbane->GetPositionX() - bot->GetPositionX());
            bot->SetFacingTo(orientation);
        }
    }

    return false;
}

bool NightbaneGroundPhaseDynamicPositionAction::Execute(Event /*event*/)
{
    if (!IsInsideNightbaneFightArea(Position(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ())))
        return false;

    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (!nightbane || nightbane->GetPositionZ() > NIGHTBANE_FLIGHT_Z)
        return false;

    if (!ShouldUseDynamicHumanTankMode(botAI, bot, nightbane))
        return false;

    MarkTargetWithSkull(bot, nightbane);
    SetRtiTarget(botAI, "skull", nightbane);
    if (bot->GetTarget() != nightbane->GetGUID())
        bot->SetTarget(nightbane->GetGUID());

    const bool isMainTank = botAI->IsMainTank(bot);
    const bool isTank = botAI->IsTank(bot);
    const bool isHealer = botAI->IsHeal(bot);
    const bool isRanged = botAI->IsRanged(bot);
    std::string role = isMainTank ? "main_tank" : (isTank ? "off_tank" : (isHealer ? "healer" : (isRanged ? "ranged" : "melee")));

    if (isMainTank)
    {
        return false;
    }

    Position anchor = GetNightbaneDynamicAnchorForBot(botAI, bot, nightbane);
    const float anchorDist = bot->GetExactDist2d(anchor.GetPositionX(), anchor.GetPositionY());
    const bool inCharredEarth = bot->HasAura(SPELL_CHARRED_EARTH);
    const bool feared = bot->HasAura(SPELL_BELLOWING_ROAR);
    Position safeAnchor;
    bool hasSafeAnchor = FindNearestSafeNightbaneAnchor(bot, nightbane, anchor, safeAnchor);
    const float zDiffRaw = std::fabs(anchor.GetPositionZ() - bot->GetPositionZ());

    if (inCharredEarth && !feared)
    {
        float escapeX = bot->GetPositionX();
        float escapeY = bot->GetPositionY();
        float escapeZ = bot->GetPositionZ();

        float dx = bot->GetPositionX() - nightbane->GetPositionX();
        float dy = bot->GetPositionY() - nightbane->GetPositionY();
        float len = std::sqrt(dx * dx + dy * dy);
        if (len < 0.1f)
        {
            dx = std::cos(nightbane->GetOrientation() + static_cast<float>(M_PI) * 0.5f);
            dy = std::sin(nightbane->GetOrientation() + static_cast<float>(M_PI) * 0.5f);
            len = 1.0f;
        }

        dx /= len;
        dy /= len;
        escapeX += dx * 6.0f;
        escapeY += dy * 6.0f;
        escapeZ = bot->GetMap()->GetHeight(escapeX, escapeY, escapeZ);
        Position wantedEscape(escapeX, escapeY, escapeZ);
        Position safeEscape;
        bool hasSafeEscape = FindNearestSafeNightbaneAnchor(bot, nightbane, wantedEscape, safeEscape);


        if (!hasSafeEscape)
        {
            return false;
        }

        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        if (!IsNightbaneMovementAllowed(bot, safeEscape))
        {
            return false;
        }
        return MoveTo(KARAZHAN_MAP_ID, safeEscape.GetPositionX(), safeEscape.GetPositionY(), safeEscape.GetPositionZ(),
                      false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    if (!hasSafeAnchor)
    {
        return false;
    }

    const float safeDist = bot->GetExactDist2d(safeAnchor.GetPositionX(), safeAnchor.GetPositionY());
    if (!IsAtNightbaneDynamicAnchor(bot, safeAnchor, isRanged || isHealer ? 3.0f : 2.0f))
    {
        if (!IsNightbaneMovementAllowed(bot, safeAnchor))
        {
            return false;
        }
        return MoveTo(KARAZHAN_MAP_ID, safeAnchor.GetPositionX(), safeAnchor.GetPositionY(), safeAnchor.GetPositionZ(),
                      false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

// Ranged bots rotate between 3 positions to avoid standing in Charred Earth, which lasts for
// 30s and has a minimum cooldown of 18s (so there can be 2 active at once)
// Ranged positions are near the Northeastern door to the tower
bool NightbaneGroundPhaseRotateRangedPositionsAction::Execute(Event /*event*/)
{
    if (!IsInsideNightbaneFightArea(Position(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ())))
        return false;

    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (nightbane && ShouldUseDynamicHumanTankMode(botAI, bot, nightbane))
    {
        return false;
    }

    const bool inCharredEarth = bot->HasAura(SPELL_CHARRED_EARTH) && !bot->HasAura(SPELL_BELLOWING_ROAR);

    if (!botAI->IsMainTank(bot) && inCharredEarth)
    {
        Position escape;
        if (FindNearestSafeNightbaneAnchor(bot, nightbane, Position(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ()), escape) &&
            bot->GetExactDist2d(escape.GetPositionX(), escape.GetPositionY()) > 1.0f &&
            IsNightbaneMovementAllowed(bot, escape))
        {
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
            return MoveTo(KARAZHAN_MAP_ID, escape.GetPositionX(), escape.GetPositionY(), escape.GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
        }
    }

    const ObjectGuid botGuid = bot->GetGUID();
    uint8 index = nightbaneRangedStep.count(botGuid) ? nightbaneRangedStep[botGuid] : 0;

    const Position rangedPositions[3] =
    {
        NIGHTBANE_RANGED_POSITION1,
        NIGHTBANE_RANGED_POSITION2,
        NIGHTBANE_RANGED_POSITION3
    };
    const Position& position = rangedPositions[index];
    const float maxDistance = 2.0f;
    float distanceToTarget = bot->GetExactDist2d(position);

    if (distanceToTarget <= maxDistance &&
        bot->HasAura(SPELL_CHARRED_EARTH) && !bot->HasAura(SPELL_BELLOWING_ROAR))
    {
        index = (index + 1) % 3;
        nightbaneRangedStep[botGuid] = index;
        const Position& newPosition = rangedPositions[index];
        float newDistanceToTarget = bot->GetExactDist2d(newPosition);
        if (newDistanceToTarget > maxDistance)
        {
            if (!IsNightbaneMovementAllowed(bot, newPosition))
            {
                return false;
            }
            bot->AttackStop();
            bot->InterruptNonMeleeSpells(true);
            return MoveTo(KARAZHAN_MAP_ID, newPosition.GetPositionX(), newPosition.GetPositionY(), newPosition.GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
        }
        return false;
    }

    if (distanceToTarget > maxDistance)
    {
        if (!IsNightbaneMovementAllowed(bot, position))
        {
            return false;
        }
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return MoveTo(KARAZHAN_MAP_ID, position.GetPositionX(), position.GetPositionY(), position.GetPositionZ(),
                      false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

// For countering Bellowing Roars during the ground phase
bool NightbaneCastFearWardOnMainTankAction::Execute(Event /*event*/)
{
    if (!IsInsideNightbaneFightArea(Position(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ())))
        return false;

    Player* mainTank = nullptr;
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && botAI->IsMainTank(member))
            {
                mainTank = member;
                break;
            }
        }
    }

    if (mainTank && botAI->CanCastSpell("fear ward", mainTank))
    {
        return botAI->CastSpell("fear ward", mainTank);
    }

    return false;
}

// Put pets on passive during the flight phase so they don't try to chase Nightbane off the map
bool NightbaneControlPetAggressionAction::Execute(Event /*event*/)
{
    if (!IsInsideNightbaneFightArea(Position(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ())))
        return false;

    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (!nightbane)
        return false;

    Pet* pet = bot->GetPet();
    if (!pet)
        return false;

    if (nightbane->GetPositionZ() <= NIGHTBANE_FLIGHT_Z && pet->GetReactState() == REACT_PASSIVE)
        pet->SetReactState(REACT_DEFENSIVE);

    if (nightbane->GetPositionZ() > NIGHTBANE_FLIGHT_Z && pet->GetReactState() != REACT_PASSIVE)
    {
        pet->AttackStop();
        pet->SetReactState(REACT_PASSIVE);
    }

    return false;
}

// 1. Stack at the "Flight Stack Position" near Nightbane so he doesn't use Fireball Barrage
// 2. Once Rain of Bones hits, the whole party moves to a new stack position
// This action lasts for the first 35 seconds of the flight phase, after which Nightbane gets
// ready to land, and the player will need to lead the bots over near the ground phase position
bool NightbaneFlightPhaseMovementAction::Execute(Event /*event*/)
{
    if (!IsInsideNightbaneFightArea(Position(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ())))
        return false;

    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (!nightbane || nightbane->GetPositionZ() <= NIGHTBANE_FLIGHT_Z)
    {
        return false;
    }


    if (AI_VALUE(Unit*, "current target") == nightbane)
    {
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
    }

    const ObjectGuid botGuid = bot->GetGUID();
    bool hasRainOfBones = bot->HasAura(SPELL_RAIN_OF_BONES);
    if (!hasRainOfBones)
    {
        if (Group* group = bot->GetGroup())
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (member && member->IsAlive() && member->HasAura(SPELL_RAIN_OF_BONES))
                {
                    hasRainOfBones = true;
                    break;
                }
            }
        }
    }

    if (hasRainOfBones)
        nightbaneRainOfBonesHit[botGuid] = true;

    float destX, destY, destZ;
    if (nightbaneRainOfBonesHit[botGuid])
    {
        destX = NIGHTBANE_RAIN_OF_BONES_POSITION.GetPositionX();
        destY = NIGHTBANE_RAIN_OF_BONES_POSITION.GetPositionY();
        destZ = NIGHTBANE_RAIN_OF_BONES_POSITION.GetPositionZ();
    }
    else
    {
        destX = NIGHTBANE_FLIGHT_STACK_POSITION.GetPositionX();
        destY = NIGHTBANE_FLIGHT_STACK_POSITION.GetPositionY();
        destZ = NIGHTBANE_FLIGHT_STACK_POSITION.GetPositionZ();
    }

    if (bot->GetExactDist2d(destX, destY) > 2.0f)
    {
        Position stackDest(destX, destY, destZ);
        if (!IsNightbaneMovementAllowed(bot, stackDest))
        {
            return false;
        }
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return MoveTo(KARAZHAN_MAP_ID, destX, destY, destZ, false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool NightbaneManageTimersAndTrackersAction::Execute(Event /*event*/)
{
    if (!IsInsideNightbaneFightArea(Position(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ())))
        return false;

    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (!nightbane)
    {
        return false;
    }

    const uint32 instanceId = nightbane->GetMap()->GetInstanceId();
    const ObjectGuid botGuid = bot->GetGUID();
    const time_t now = std::time(nullptr);
    const bool isGround = nightbane->GetPositionZ() <= NIGHTBANE_FLIGHT_Z;
    const bool wasInFlight = nightbaneWasInFlightPhase.count(instanceId) ? nightbaneWasInFlightPhase[instanceId] : false;

    if (!IsInsideNightbaneFightArea(Position(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ())))
    {
        bot->AttackStop();
        if (MotionMaster* mm = bot->GetMotionMaster())
            mm->Clear(false);

        Position safe = GetNearestNightbaneSafePoint(bot);
        if (IsNightbaneMovementAllowed(bot, safe))
        {
            return MoveTo(KARAZHAN_MAP_ID, safe.GetPositionX(), safe.GetPositionY(), safe.GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
        }

        return false;
    }

    // Erase DPS wait timer and tank and ranged position tracking on encounter reset
    if (nightbane->GetHealth() == nightbane->GetMaxHealth())
    {
        if (botAI->IsMainTank(bot))
            nightbaneTankStep.erase(botGuid);

        if (botAI->IsRanged(bot))
            nightbaneRangedStep.erase(botGuid);

        if (IsMechanicTrackerBot(botAI, bot, KARAZHAN_MAP_ID, nullptr))
            nightbaneDpsWaitTimer.erase(instanceId);
        nightbaneWasInFlightPhase.erase(instanceId);
    }
    // Erase flight phase timer and Rain of Bones tracker on ground phase and start DPS wait timer
    else if (isGround)
    {
        const bool justLanded = wasInFlight;
        nightbaneRainOfBonesHit.erase(botGuid);

        if (Group* group = bot->GetGroup())
        {
            if (group->GetTargetIcon(RtiTargetValue::moonIndex) == nightbane->GetGUID())
            {
                group->SetTargetIcon(RtiTargetValue::moonIndex, bot->GetGUID(), ObjectGuid::Empty);
            }
        }

        MarkTargetWithSkull(bot, nightbane);
        SetRtiTarget(botAI, "skull", nightbane);
        if (bot->GetTarget() != nightbane->GetGUID())
            bot->SetTarget(nightbane->GetGUID());

        nightbaneFlightPhaseStartTimer.erase(instanceId);
        if (justLanded)
            nightbaneDpsWaitTimer[instanceId] = now;
        else
            nightbaneDpsWaitTimer.try_emplace(instanceId, now);
        nightbaneWasInFlightPhase[instanceId] = false;
    }
    // Erase DPS wait timer and tank and ranged position tracking and start flight phase timer
    // at beginning of flight phase
    else if (nightbane->GetPositionZ() > NIGHTBANE_FLIGHT_Z)
    {
        if (botAI->IsMainTank(bot))
            nightbaneTankStep.erase(botGuid);

        if (botAI->IsRanged(bot))
            nightbaneRangedStep.erase(botGuid);

        nightbaneDpsWaitTimer.erase(instanceId);
        nightbaneFlightPhaseStartTimer.try_emplace(instanceId, now);
        nightbaneWasInFlightPhase[instanceId] = true;
    }

    return false;
}
