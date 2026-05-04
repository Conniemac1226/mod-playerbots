#include "RaidKarazhanActions.h"
#include "RaidKarazhanHelpers.h"
#include "Playerbots.h"
#include "PlayerbotTextMgr.h"
#include "RaidBossHelpers.h"
#include "RtiTargetValue.h"
#include "MotionMaster.h"
#include <algorithm>
#include <ctime>
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

        if (bot->GetTarget() != attumenMounted->GetGUID())
        {
            bot->SetTarget(attumenMounted->GetGUID());
            return Attack(attumenMounted);
        }
    }
    else if (Unit* midnight = AI_VALUE2(Unit*, "find target", "midnight"))
    {
        if (IsMechanicTrackerBot(botAI, bot, KARAZHAN_MAP_ID, nullptr))
            MarkTargetWithStar(bot, midnight);

        if (!botAI->IsAssistTankOfIndex(bot, 0))
        {
            SetRtiTarget(botAI, "star", midnight);

            if (bot->GetTarget() != midnight->GetGUID())
            {
                bot->SetTarget(midnight->GetGUID());
                return Attack(midnight);
            }
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

    if (bot->GetVictim() != attumen)
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

    if (bot->GetVictim() != moroes)
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

    if (bot->GetVictim() != maiden)
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

    if (bot->GetVictim() != wolf)
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

    if (bot->GetVictim() != curator)
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
    std::unordered_map<ObjectGuid, time_t> chessMovementCooldownUntilByPiece;
    std::unordered_map<uint32, std::set<ObjectGuid>> chessOpenedLanePawnsByInstance;

    enum class ChessPhase : uint8
    {
        OPENING = 0,
        CLAIM_HIGH_VALUE = 1,
        COMBAT = 2
    };

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
    };

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
        if (!botAI || !bot)
            return state;

        std::vector<Creature*> triggers = GetNearbyChessMoveTriggers(botAI, bot);
        if (triggers.empty())
            return state;

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
            if (!WorldToChessSquare(state, p->GetPositionX(), p->GetPositionY(), row, col))
                continue;
            if (row < state.minRow || row > state.maxRow || col < state.minCol || col > state.maxCol)
                continue;
            ChessSquare sq{ row, col };
            state.pieceSquare[p->GetGUID()] = sq;
            state.occupied.insert({ row, col });
        }

        return state;
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
        Creature* enemyKing = GetEnemyChessKing(botAI, bot);
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
        std::vector<Creature*> friendly = GetNearbyChessPieces(botAI, bot, true);

        uint32 movedPawns = 0;
        uint32 friendlyPawns = 0;
        uint32 blockedHighValue = 0;
        for (Creature* p : friendly)
        {
            if (!p)
                continue;
            if (IsPawnEntry(p->GetEntry()))
            {
                ++friendlyPawns;
                if (chessOpenedLanePawnsByInstance[instanceId].find(p->GetGUID()) != chessOpenedLanePawnsByInstance[instanceId].end())
                    ++movedPawns;
                continue;
            }

            std::string legalDbg;
            std::vector<Creature*> legal = GetLegalMoveTriggersForPiece(botAI, bot, p, b, legalDbg);
            if (legal.empty())
                ++blockedHighValue;
        }

        const bool openingDoneByPawns = movedPawns >= std::min<uint32>(4, friendlyPawns);
        const bool openingDoneByAccess = blockedHighValue == 0;
        if (!openingDoneByPawns && !openingDoneByAccess)
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
    if (!IsChessEventActive(botAI, bot))
        return false;

    const uint32 instanceId = bot->GetMap() ? bot->GetMap()->GetInstanceId() : 0;
    const time_t now = std::time(nullptr);
    if (instanceId)
        chessEventStartByInstance.try_emplace(instanceId, now);
    ChessBoardState board = BuildChessBoardState(botAI, bot);
    ChessPhase phase = GetChessPhase(botAI, bot, board);
    const bool canClaimKing = botAI->IsMainTank(bot) || botAI->IsTank(bot);

    ChessSide botSide = GetChessSideForBot(bot);
    if (botSide == ChessSide::UNKNOWN)
    {
        LogKarazhanChessDebug(bot, "claim skipped: side UNKNOWN");
        return false;
    }

    Creature* charm = bot->GetCharm() ? bot->GetCharm()->ToCreature() : nullptr;
    if (charm && IsChessPieceEntry(charm->GetEntry()))
        return false;

    Creature* assigned = GetAssignedChessPiece(bot);
    if (assigned && assigned->IsAlive())
    {
        if (!IsPawnEntry(assigned->GetEntry()))
        {
            LogKarazhanChessDebug(bot, "claim assigned cleared: pawn-only mode");
            ClearAssignedChessPiece(bot);
            return false;
        }

        if (phase == ChessPhase::OPENING && !IsPawnEntry(assigned->GetEntry()))
        {
            LogKarazhanChessDebug(bot, "claim assigned cleared: opening pawn phase");
            ClearAssignedChessPiece(bot);
            return false;
        }

        if (IsKingEntry(assigned->GetEntry()) && !canClaimKing)
        {
            LogKarazhanChessDebug(bot, "claim assigned cleared: king restricted to tank");
            ClearAssignedChessPiece(bot);
            return false;
        }

        if (!IsClaimableFriendlyPawnForOpening(bot, assigned))
        {
            LogKarazhanChessDebug(
                bot, "claim assigned rejected side=" + ChessSideToString(botSide) +
                " piece=" + assigned->GetName() + " entry=" + std::to_string(assigned->GetEntry()) +
                " classified=" + PieceSideToString(bot, assigned));
            ClearAssignedChessPiece(bot);
            return false;
        }
        float dist = bot->GetExactDist2d(assigned);
        if (dist > 8.0f)
        {
            LogKarazhanChessDebug(bot, "claim assigned piece move-in dist=" + std::to_string(dist));
            return MoveTo(KARAZHAN_MAP_ID, assigned->GetPositionX(), assigned->GetPositionY(), assigned->GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
        }
        bot->CastSpell(assigned, SPELL_CONTROL_PIECE, true);
        LogKarazhanChessDebug(bot, "claim assigned piece=" + assigned->GetName());
        return true;
    }

    std::vector<Creature*> nearby = GetNearbyChessPieces(botAI, bot, false);
    if (nearby.empty())
    {
        LogKarazhanChessDebug(bot, "claim skipped: no nearby pieces");
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
        bool const pawnOnlyMode = true;
        bool finalClaimAllowed = false;
        std::string rejectReason;
        ChessSide const pieceBotSide = GetChessSideForBot(bot);
        bool const sideKnown = pieceBotSide != ChessSide::UNKNOWN;
        bool const isFriendlyForBot = IsFriendlyChessPieceForBot(bot, piece);
        bool const alive = piece->IsAlive();
        bool const charmed = piece->IsCharmed();
        bool const openingClaimable = IsClaimableFriendlyPawnForOpening(bot, piece);

        if (!openingClaimable)
        {
            if (!sideKnown)
                rejectReason = "side-unknown";
            else if (!isFriendlyForBot)
                rejectReason = "wrong-side";
            else if (!isPawn)
                rejectReason = "not-pawn";
            else if (!alive)
                rejectReason = "dead";
            else
                rejectReason = "not-claimable";
        }
        else if (IsKingEntry(piece->GetEntry()) && !canClaimKing)
            rejectReason = "king-restricted-to-tank";
        else if (IsPieceAssignedToOtherBot(bot, piece))
            rejectReason = "assigned-to-other-bot";
        else if (pawnOnlyMode && !isPawn)
            rejectReason = "pawn-only-mode";
        else if (phase == ChessPhase::OPENING && hasClaimablePawn && !isPawn)
            rejectReason = "opening-pawn-phase";
        else
            finalClaimAllowed = true;

        LogKarazhanChessDebug(
            bot, "claim consider side=" + ChessSideToString(botSide) +
            " piece=" + piece->GetName() + " entry=" + std::to_string(piece->GetEntry()) +
            " classified=" + PieceSideToString(bot, piece) +
            " isPawn=" + std::string(isPawn ? "1" : "0") +
            " alive=" + std::string(alive ? "1" : "0") +
            " charmed=" + std::string(charmed ? "1" : "0") +
            " sideKnown=" + std::string(sideKnown ? "1" : "0") +
            " isFriendlyForBot=" + std::string(isFriendlyForBot ? "1" : "0") +
            " pawnOnlyMode=" + std::string(pawnOnlyMode ? "1" : "0") +
            " finalClaimAllowed=" + std::string(finalClaimAllowed ? "1" : "0") +
            " rejectReason=" + (rejectReason.empty() ? "none" : rejectReason));

        if (!finalClaimAllowed)
            continue;

        // Hard guard requested: never select non-claimable targets.
        if (!IsClaimableFriendlyPawnForOpening(bot, piece))
        {
            LogKarazhanChessDebug(bot, "claim rejected wrong-side/unknown piece=" + piece->GetName());
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
        LogKarazhanChessDebug(bot, "claim skipped: no claimable piece selected");
        return false;
    }

    float bestDist = bot->GetExactDist2d(best);
    if (bestDist > 8.0f)
    {
        LogKarazhanChessDebug(bot, "claim selected piece move-in piece=" + best->GetName() + " dist=" + std::to_string(bestDist));
        return MoveTo(KARAZHAN_MAP_ID, best->GetPositionX(), best->GetPositionY(), best->GetPositionZ(),
                      false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    bot->CastSpell(best, SPELL_CONTROL_PIECE, true);
    LogKarazhanChessDebug(bot, "claim piece=" + best->GetName() + " guid=" + best->GetGUID().ToString());
    return true;
}

bool KarazhanChessMovePieceAction::Execute(Event /*event*/)
{
    Creature* piece = bot->GetCharm() ? bot->GetCharm()->ToCreature() : nullptr;
    if (!piece || !IsChessPieceEntry(piece->GetEntry()))
        return false;
    if (!IsPawnEntry(piece->GetEntry()))
        return false;
    if (!IsChessPieceCommandGcdReady(piece))
    {
        LogKarazhanChessDebug(bot, "decision=HOLD reason=piece_global_cooldown_or_casting");
        return false;
    }
    const time_t now = std::time(nullptr);
    if (!IsChessMoveReady(piece, now, 5))
    {
        LogKarazhanChessDebug(bot, "decision=HOLD reason=move_window_or_move_cooldown");
        return false;
    }
    const uint32 instanceId = bot->GetMap() ? bot->GetMap()->GetInstanceId() : 0;

    Creature* enemyKing = GetEnemyChessKing(botAI, bot);
    if (!enemyKing)
    {
        LogKarazhanChessDebug(bot, "decision=HOLD reason=no_enemy_king_live_state");
        return false;
    }

    ChessBoardState board = BuildChessBoardState(botAI, bot);
    ChessPhase phase = GetChessPhase(botAI, bot, board);
    auto pieceSqIt = board.pieceSquare.find(piece->GetGUID());
    if (pieceSqIt != board.pieceSquare.end())
        chessLastSquareByPiece[piece->GetGUID()] = pieceSqIt->second;
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
            LogKarazhanChessDebug(
                bot, "king-refresh enemy_guid=" + enemyKing->GetGUID().ToString() +
                " enemy_name=" + enemyKing->GetName() +
                " enemy_pos=(" + std::to_string(enemyKing->GetPositionX()) + "," +
                std::to_string(enemyKing->GetPositionY()) + "," + std::to_string(enemyKing->GetPositionZ()) + ")" +
                " enemy_square=(" + std::to_string(current.row) + "," + std::to_string(current.col) + ")" +
                " prev_square=" + prev +
                " board_refreshed=1");
            chessLastEnemyKingSquareByBot[bot->GetGUID()] = current;
        }
    }
    else
    {
        LogKarazhanChessDebug(
            bot, "king-refresh enemy_guid=" + enemyKing->GetGUID().ToString() +
            " enemy_name=" + enemyKing->GetName() +
            " enemy_pos=(" + std::to_string(enemyKing->GetPositionX()) + "," +
            std::to_string(enemyKing->GetPositionY()) + "," + std::to_string(enemyKing->GetPositionZ()) + ")" +
            " enemy_square=unknown board_refreshed=1");
    }

    std::string legalDbg;
    std::vector<Creature*> triggers = GetLegalMoveTriggersForPiece(botAI, bot, piece, board, legalDbg);
    uint32 const moveSpell = GetChessMoveSpellForPiece(piece);
    std::string const pieceSpellList = GetChessPieceSpellList(piece);
    std::vector<std::pair<float, Creature*>> legalMoves;
    ObjectGuid const pieceGuid = piece->GetGUID();
    time_t const nowTick = now;
    auto moveIt = chessLastMoveCommandByPiece.find(pieceGuid);
    if (moveIt != chessLastMoveCommandByPiece.end() && (nowTick - moveIt->second) < 2)
    {
        LogKarazhanChessDebug(bot, "decision=HOLD reason=move_throttled");
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
        LogKarazhanChessDebug(
            bot, "decision=HOLD reason=no_legal_move_trigger_or_only_recently_failed piece=" + piece->GetName() +
            " entry=" + std::to_string(piece->GetEntry()) +
            " world=(" + std::to_string(piece->GetPositionX()) + "," + std::to_string(piece->GetPositionY()) + "," + std::to_string(piece->GetPositionZ()) + ")" +
            " board_origin=(" + std::to_string(board.originX) + "," + std::to_string(board.originY) + ")" +
            " board_cell_row=(" + std::to_string(board.rowStepX) + "," + std::to_string(board.rowStepY) + ")" +
            " board_cell_col=(" + std::to_string(board.colStepX) + "," + std::to_string(board.colStepY) + ")" +
            " board_bounds=" + squareBounds +
            " current_square=" + curSq +
            " forward=" + std::to_string(forward) +
            " legal=" + legalDbg +
            " piece_spells=" + pieceSpellList +
            " selected_movement_spell=" + std::to_string(moveSpell) +
            " movement_spell_found=" + std::string(hasMoveSpell ? "1" : "0") +
            " movement_spell_cooldown=" + std::string((hasMoveSpell && piece->HasSpellCooldown(moveSpell)) ? "1" : "0"));
        return false;
    }

    std::sort(legalMoves.begin(), legalMoves.end(),
        [](std::pair<float, Creature*> const& lhs, std::pair<float, Creature*> const& rhs)
        {
            return lhs.first < rhs.first;
        });

    // Pawn-only deterministic opener: one legal forward/outward pawn move via native chess move spell.
    if (IsPawnEntry(piece->GetEntry()))
    {
        if (pieceSqIt == board.pieceSquare.end())
        {
            LogKarazhanChessDebug(bot, "decision=HOLD reason=opening_pawn_no_current_square");
            return false;
        }
        ChessSquare curSq = pieceSqIt->second;
        int bestForward = 0;
        Creature* bestForwardTrigger = nullptr;
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
            if (absForward > bestForward)
            {
                bestForward = absForward;
                bestForwardTrigger = mv.second;
            }
        }

        if (!bestForwardTrigger)
        {
            LogKarazhanChessDebug(bot, "decision=HOLD reason=pawn_no_forward_legal_move phase=" + ChessPhaseToString(phase) +
                                       " candidates=" + candDbg);
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
            LogKarazhanChessDebug(
                bot, "decision=HOLD pawn=" + piece->GetName() + " guid=" + piece->GetGUID().ToString() +
                " current=(" + std::to_string(curSq.row) + "," + std::to_string(curSq.col) + ")" +
                " dest=(" + std::to_string(toRow) + "," + std::to_string(toCol) + ")" +
                " occupied=1 occupied_by=" + occBy + " reason=destination_occupied");
            return false;
        }

        if (!moveSpell)
        {
            LogKarazhanChessDebug(
                bot, "decision=HOLD pawn=" + piece->GetName() + " entry=" + std::to_string(piece->GetEntry()) +
                " current=(" + std::to_string(curSq.row) + "," + std::to_string(curSq.col) + ")" +
                " dest=(" + std::to_string(toRow) + "," + std::to_string(toCol) + ")" +
                " piece_spells=" + pieceSpellList +
                " selected_movement_spell=0 movement_spell_found=0 reason=no_movement_spell_on_piece");
            return false;
        }

        piece->CastSpell(bestForwardTrigger, moveSpell, true);
        if (piece->HasAura(SPELL_MOVE_COOLDOWN))
        {
            StampChessPieceCommandGcd(piece);
            chessLastFailedMoveTriggerByPiece.erase(pieceGuid);
            chessLastFailedMoveTimeByPiece.erase(pieceGuid);
            chessLastMoveCommandByPiece[pieceGuid] = nowTick;
            chessMovementCooldownUntilByPiece[pieceGuid] = nowTick + 5;
            chessOpenedLanePawnsByInstance[instanceId].insert(piece->GetGUID());
            chessLastMoveFromSquareByPiece[pieceGuid] = curSq;
            chessLastMoveToSquareByPiece[pieceGuid] = ChessSquare{ toRow, toCol };
            LogKarazhanChessDebug(
                bot, "decision=MOVE pawn=" + piece->GetName() + " guid=" + piece->GetGUID().ToString() +
                " current=(" + std::to_string(curSq.row) + "," + std::to_string(curSq.col) + ")" +
                " dest=(" + std::to_string(toRow) + "," + std::to_string(toCol) + ")" +
                " occupied=0 move_spell=" + std::to_string(moveSpell) +
                " gcd_ms=1500 move_cd_sec=5 result=success");
            return true;
        }

        LogKarazhanChessDebug(
            bot, "decision=HOLD pawn=" + piece->GetName() + " guid=" + piece->GetGUID().ToString() +
            " current=(" + std::to_string(curSq.row) + "," + std::to_string(curSq.col) + ")" +
            " dest=(" + std::to_string(toRow) + "," + std::to_string(toCol) + ")" +
            " occupied=0 move_spell=" + std::to_string(moveSpell) +
            " result=fail reason=move_not_applied");
        return false;
    }

    LogKarazhanChessDebug(bot, "decision=HOLD reason=non_pawn_move_blocked");
    return false;
}

bool KarazhanChessMoveOutOfFireAction::Execute(Event /*event*/)
{
    Creature* piece = bot->GetCharm() ? bot->GetCharm()->ToCreature() : nullptr;
    if (!piece || !IsChessPieceEntry(piece->GetEntry()))
        return false;
    if (!IsPawnEntry(piece->GetEntry()))
        return false;
    if (!IsChessPieceCommandGcdReady(piece))
    {
        LogKarazhanChessDebug(bot, "decision=HOLD reason=piece_global_cooldown_or_casting");
        return false;
    }
    const time_t now = std::time(nullptr);
    if (!IsChessMoveReady(piece, now, 5))
    {
        LogKarazhanChessDebug(bot, "decision=HOLD reason=move_window_or_move_cooldown");
        return false;
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
        LogKarazhanChessDebug(bot, "decision=HOLD move-out-of-fire skipped: no safe tile");
        return false;
    }

    if (!moveSpell)
    {
        LogKarazhanChessDebug(bot, "decision=HOLD move-out-of-fire skipped: no movement spell piece_spells=" + GetChessPieceSpellList(piece));
        return false;
    }

    piece->CastSpell(safe, moveSpell, true);
    if (piece->HasAura(SPELL_MOVE_COOLDOWN))
    {
        StampChessPieceCommandGcd(piece);
        chessLastMoveCommandByPiece[piece->GetGUID()] = now;
    }
    LogKarazhanChessDebug(bot, "decision=MOVE_SAFE move-out-of-fire spell=" + std::to_string(moveSpell) +
                               " trigger=" + safe->GetGUID().ToString());
    return true;
}

bool KarazhanChessUseAbilityAction::Execute(Event /*event*/)
{
    Creature* piece = bot->GetCharm() ? bot->GetCharm()->ToCreature() : nullptr;
    if (!piece || !IsChessPieceEntry(piece->GetEntry()))
        return false;
    if (!IsPawnEntry(piece->GetEntry()))
        return false;
    if (!IsChessPieceCommandGcdReady(piece))
    {
        LogKarazhanChessDebug(bot, "decision=HOLD reason=piece_global_cooldown_or_casting");
        return false;
    }

    Creature* friendlyKing = GetFriendlyChessKing(botAI, bot);
    Creature* enemyKing = GetEnemyChessKing(botAI, bot);
    const bool canMove = !piece->HasAura(SPELL_MOVE_COOLDOWN);
    const bool needsMovement = canMove && enemyKing && piece->GetExactDist2d(enemyKing) > 22.0f;
    const bool healerNeedsTarget = IsHealerChessPieceEntry(piece->GetEntry()) && friendlyKing && friendlyKing->GetHealthPct() < 90.0f;
    const bool hasValidChessTarget = healerNeedsTarget || enemyKing;
    const time_t now = std::time(nullptr);
    const ObjectGuid pieceGuid = piece->GetGUID();
    const bool isPawn = piece->GetEntry() == NPC_PAWN_A || piece->GetEntry() == NPC_PAWN_H;
    const float kingDist = enemyKing ? piece->GetExactDist2d(enemyKing) : 999.0f;
    const uint32 instanceId = bot->GetMap() ? bot->GetMap()->GetInstanceId() : 0;
    ChessBoardState board = BuildChessBoardState(botAI, bot);
    ChessPhase phase = GetChessPhase(botAI, bot, board);
    bool const openingPawnMoved = instanceId && chessOpenedLanePawnsByInstance.count(instanceId) &&
        !chessOpenedLanePawnsByInstance[instanceId].empty();

    if (!IsChessAbilityReady(piece, now, 5))
    {
        LogKarazhanChessDebug(bot, "decision=HOLD reason=ability_window");
        return false;
    }

    // Opening behavior: pawns should clear lanes before spamming abilities.
    if (isPawn && canMove && kingDist > 14.0f)
    {
        LogKarazhanChessDebug(bot, "decision=HOLD reason=pawn_prioritize_move");
        return false;
    }

    // Keep visual spam down and prioritize board progress when still far from enemy king.
    if (!isPawn && canMove && kingDist > 20.0f && !healerNeedsTarget)
    {
        LogKarazhanChessDebug(bot, "decision=HOLD reason=far_from_engagement_prioritize_move");
        return false;
    }

    for (uint8 i = 0; i < MAX_CREATURE_SPELLS; ++i)
    {
        uint32 spellId = piece->m_spells[i];
        if (!spellId || piece->HasSpellCooldown(spellId))
            continue;

        if (healerNeedsTarget)
        {
            chessLastAbilityCommandByPiece[pieceGuid] = now;
            piece->CastSpell(friendlyKing, spellId, true);
            if (piece->HasSpellCooldown(spellId))
            {
                StampChessPieceCommandGcd(piece);
                chessLastAbilityCommandByPiece[pieceGuid] = now;
                LogKarazhanChessDebug(bot, "decision=HEAL selected spell id=" + std::to_string(spellId) +
                                           " target=" + friendlyKing->GetName());
                return true;
            }
            LogKarazhanChessDebug(bot, "decision=HEAL skipped spell id=" + std::to_string(spellId) +
                                       " target=" + friendlyKing->GetName() + " reason=cast_not_applied");
            continue;
        }

        if (enemyKing)
        {
            if (phase == ChessPhase::OPENING && !openingPawnMoved)
            {
                LogKarazhanChessDebug(bot, "decision=HOLD reason=opening_no_attack_until_pawn_moves");
                continue;
            }
            chessLastAbilityCommandByPiece[pieceGuid] = now;
            piece->CastSpell(enemyKing, spellId, true);
            if (piece->HasSpellCooldown(spellId))
            {
                StampChessPieceCommandGcd(piece);
                chessLastAbilityCommandByPiece[pieceGuid] = now;
                LogKarazhanChessDebug(bot, "decision=ATTACK_KING selected spell id=" + std::to_string(spellId) +
                                           " target=" + enemyKing->GetName());
                return true;
            }
            LogKarazhanChessDebug(bot, "decision=ATTACK_KING skipped spell id=" + std::to_string(spellId) +
                                       " target=" + enemyKing->GetName() + " reason=cast_not_applied");
            continue;
        }

        if (needsMovement || hasValidChessTarget)
        {
            LogKarazhanChessDebug(bot, "decision=SELF_BUFF skip self spell id=" + std::to_string(spellId) +
                                       " reason=movement_or_target_priority");
            continue;
        }

        time_t& selfThrottle = chessSelfAbilityThrottleByPiece[piece->GetGUID()];
        if ((now - selfThrottle) < 8)
        {
            LogKarazhanChessDebug(bot, "decision=SELF_BUFF skip self spell id=" + std::to_string(spellId) +
                                       " reason=throttled");
            continue;
        }

        piece->CastSpell(piece, spellId, true);
        chessLastAbilityCommandByPiece[pieceGuid] = now;
        if (piece->HasSpellCooldown(spellId))
        {
            selfThrottle = now;
            StampChessPieceCommandGcd(piece);
            chessLastAbilityCommandByPiece[pieceGuid] = now;
            LogKarazhanChessDebug(bot, "decision=SELF_BUFF ability self spell=" + std::to_string(spellId));
            return true;
        }
        LogKarazhanChessDebug(bot, "decision=SELF_BUFF skipped spell id=" + std::to_string(spellId) +
                                   " reason=cast_not_applied");
    }

    LogKarazhanChessDebug(bot, "decision=HOLD reason=no_ability_applied");
    return false;
}

bool KarazhanChessHealFriendlyAction::Execute(Event event)
{
    (void)event;
    Creature* piece = bot->GetCharm() ? bot->GetCharm()->ToCreature() : nullptr;
    Creature* king = GetFriendlyChessKing(botAI, bot);
    if (!piece || !king || !IsHealerChessPieceEntry(piece->GetEntry()))
        return false;
    if (!IsPawnEntry(piece->GetEntry()))
        return false;
    if (!IsChessPieceCommandGcdReady(piece))
        return false;
    const time_t now = std::time(nullptr);
    if (!IsChessAbilityReady(piece, now, 5))
        return false;

    for (uint8 i = 0; i < MAX_CREATURE_SPELLS; ++i)
    {
        uint32 spellId = piece->m_spells[i];
        if (spellId && !piece->HasSpellCooldown(spellId))
        {
            chessLastAbilityCommandByPiece[piece->GetGUID()] = now;
            piece->CastSpell(king, spellId, true);
            if (piece->HasSpellCooldown(spellId))
                StampChessPieceCommandGcd(piece);
            LogKarazhanChessDebug(bot, "decision=HEAL heal-friendly spell=" + std::to_string(spellId) + " target=" + king->GetName());
            return true;
        }
    }

    return false;
}

bool KarazhanChessAttackEnemyKingAction::Execute(Event /*event*/)
{
    Creature* piece = bot->GetCharm() ? bot->GetCharm()->ToCreature() : nullptr;
    Creature* enemyKing = GetEnemyChessKing(botAI, bot);
    if (!piece || !enemyKing)
        return false;
    if (!IsPawnEntry(piece->GetEntry()))
        return false;
    if (!IsChessPieceCommandGcdReady(piece))
        return false;
    const time_t now = std::time(nullptr);
    if (!IsChessAbilityReady(piece, now, 5))
        return false;
    const uint32 instanceId = bot->GetMap() ? bot->GetMap()->GetInstanceId() : 0;
    ChessBoardState board = BuildChessBoardState(botAI, bot);
    ChessPhase phase = GetChessPhase(botAI, bot, board);
    bool const openingPawnMoved = instanceId && chessOpenedLanePawnsByInstance.count(instanceId) &&
        !chessOpenedLanePawnsByInstance[instanceId].empty();
    if (phase == ChessPhase::OPENING && !openingPawnMoved)
    {
        LogKarazhanChessDebug(bot, "decision=HOLD reason=opening_no_attack_until_pawn_moves");
        return false;
    }

    for (uint8 i = 0; i < MAX_CREATURE_SPELLS; ++i)
    {
        uint32 spellId = piece->m_spells[i];
        if (spellId && !piece->HasSpellCooldown(spellId))
        {
            chessLastAbilityCommandByPiece[piece->GetGUID()] = now;
            piece->CastSpell(enemyKing, spellId, true);
            if (piece->HasSpellCooldown(spellId))
                StampChessPieceCommandGcd(piece);
            LogKarazhanChessDebug(bot, "decision=ATTACK_KING attack-king spell=" + std::to_string(spellId) +
                                       " target=" + enemyKing->GetName());
            return true;
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
    if (!IsChessPieceCommandGcdReady(piece))
        return false;
    const time_t now = std::time(nullptr);
    if (!IsChessMoveReady(piece, now, 5))
        return false;

    Creature* king = GetFriendlyChessKing(botAI, bot);
    Creature* enemyKing = GetEnemyChessKing(botAI, bot);
    if (!king || !enemyKing)
        return false;

    float midX = (king->GetPositionX() + enemyKing->GetPositionX()) * 0.5f;
    float midY = (king->GetPositionY() + enemyKing->GetPositionY()) * 0.5f;

    ChessBoardState board = BuildChessBoardState(botAI, bot);
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
        LogKarazhanChessDebug(bot, "decision=HOLD block-path skipped: no movement spell piece_spells=" + GetChessPieceSpellList(piece));
        return false;
    }

    piece->CastSpell(best, moveSpell, true);
    if (piece->HasAura(SPELL_MOVE_COOLDOWN))
    {
        StampChessPieceCommandGcd(piece);
        chessLastMoveCommandByPiece[piece->GetGUID()] = now;
    }
    LogKarazhanChessDebug(bot, "decision=BLOCK block-path move spell=" + std::to_string(moveSpell) +
                               " trigger=" + best->GetGUID().ToString());
    return true;
}

bool KarazhanChessReleaseOrReassignAction::Execute(Event /*event*/)
{
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
            chessLastSquareByPiece.erase(pg);
            chessLastMoveFromSquareByPiece.erase(pg);
            chessLastMoveToSquareByPiece.erase(pg);
            chessLastMoveCommandByPiece.erase(pg);
            chessLastAbilityCommandByPiece.erase(pg);
            chessMovementCooldownUntilByPiece.erase(pg);
        }
        ClearAssignedChessPiece(bot);
        return false;
    }

    Creature* charm = bot->GetCharm() ? bot->GetCharm()->ToCreature() : nullptr;
    if (charm && !IsChessPieceEntry(charm->GetEntry()))
    {
        chessSelfAbilityThrottleByPiece.erase(charm->GetGUID());
        chessLastEnemyKingSquareByBot.erase(bot->GetGUID());
        ClearAssignedChessPiece(bot);
        return false;
    }

    Creature* assigned = GetAssignedChessPiece(bot);
    if (assigned && (!assigned->IsAlive() || assigned->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE)))
    {
        LogKarazhanChessDebug(bot, "assigned piece invalid, clearing");
        chessSelfAbilityThrottleByPiece.erase(assigned->GetGUID());
        ClearAssignedChessPiece(bot);
    }
    else if (assigned)
    {
        ChessBoardState board = BuildChessBoardState(botAI, bot);
        ChessPhase phase = GetChessPhase(botAI, bot, board);
        bool const pawnOnlyMode = true;
        bool const isPawn = IsPawnEntry(assigned->GetEntry());
        bool const isFriendlyForBot = IsFriendlyChessPieceForBot(bot, assigned);
        bool const openingAssignedAllowed = IsClaimableFriendlyPawnForOpening(bot, assigned);
        bool const assignedAllowed = openingAssignedAllowed && (!pawnOnlyMode || isPawn);

        if (!assignedAllowed)
        {
            Unit* controller = assigned->GetCharmerOrOwner();
            std::string clearReason = !isFriendlyForBot ? "wrong-side" : (!isPawn ? "non-pawn-in-opening" : "not-claimable");
            LogKarazhanChessDebug(
                bot, "assigned validation guid=" + assigned->GetGUID().ToString() +
                " entry=" + std::to_string(assigned->GetEntry()) +
                " name=" + assigned->GetName() +
                " side=" + ChessSideToString(GetChessSideForBot(bot)) +
                " isPawn=" + std::string(isPawn ? "1" : "0") +
                " isFriendlyForBot=" + std::string(isFriendlyForBot ? "1" : "0") +
                " pawnOnlyMode=" + std::string(pawnOnlyMode ? "1" : "0") +
                " charmed=" + std::string(assigned->IsCharmed() ? "1" : "0") +
                " controller=" + (controller ? controller->GetName() : std::string("none")) +
                " finalAssignedAllowed=" + std::string(assignedAllowed ? "1" : "0") +
                " clearReason=" + clearReason +
                " phase=" + ChessPhaseToString(phase));
            chessSelfAbilityThrottleByPiece.erase(assigned->GetGUID());
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

    if (bot->GetVictim() != malchezaar)
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
    if (!IsKarazhanNightbaneEnabled())
        return false;

    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (!nightbane)
    {
        LogKarazhanNightbaneDebug(bot, "ground-phase: no target");
        return false;
    }
    if (ShouldUseDynamicHumanTankMode(botAI, bot, nightbane))
    {
        LogKarazhanNightbaneDebug(bot, "ground-phase fixed boss positioning skipped: human-tank mode active");
        return false;
    }

    MarkTargetWithSkull(bot, nightbane);
    SetRtiTarget(botAI, "skull", nightbane);

    if (bot->GetVictim() != nightbane)
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
                LogKarazhanNightbaneDebug(
                    bot, "Nightbane containment reject: requested destination outside allowed area/path"
                    " dest=(" + std::to_string(position.GetPositionX()) + "," + std::to_string(position.GetPositionY()) + "," +
                    std::to_string(position.GetPositionZ()) + ")"
                    " bot=(" + std::to_string(bot->GetPositionX()) + "," + std::to_string(bot->GetPositionY()) + "," +
                    std::to_string(bot->GetPositionZ()) + ")");
                return false;
            }
            LogKarazhanNightbaneDebug(
                bot, "ground-phase: moving step=" + std::to_string(step) +
                " dist=" + std::to_string(distanceToTarget));
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
    if (!IsKarazhanNightbaneEnabled())
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
        LogKarazhanNightbaneDebug(bot, "dynamic-ground role=main_tank skipping movement authority to human tank");
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

        LogKarazhanNightbaneDebug(
            bot, "dynamic-ground mode=charred-earth role=" + role +
            " boss=(" + std::to_string(nightbane->GetPositionX()) + "," + std::to_string(nightbane->GetPositionY()) + "," +
            std::to_string(nightbane->GetPositionZ()) + ") face=" + std::to_string(nightbane->GetOrientation()) +
            " anchor=(" + std::to_string(anchor.GetPositionX()) + "," + std::to_string(anchor.GetPositionY()) + "," +
            std::to_string(anchor.GetPositionZ()) + ") dist=" + std::to_string(anchorDist) +
            " raw_move_to=(" + std::to_string(escapeX) + "," + std::to_string(escapeY) + "," + std::to_string(escapeZ) + ")" +
            " safe=" + std::string(hasSafeEscape ? "1" : "0"));

        if (!hasSafeEscape)
        {
            LogKarazhanNightbaneDebug(
                bot, "Nightbane: rejected unsafe anchor, holding current position."
                " reason=charred-earth no safe escape"
                " bot=(" + std::to_string(bot->GetPositionX()) + "," + std::to_string(bot->GetPositionY()) + "," +
                std::to_string(bot->GetPositionZ()) + ") zdiff=" + std::to_string(std::fabs(wantedEscape.GetPositionZ() - bot->GetPositionZ())));
            return false;
        }

        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        if (!IsNightbaneMovementAllowed(bot, safeEscape))
        {
            LogKarazhanNightbaneDebug(bot, "Nightbane: rejected unsafe anchor, holding current position. reason=escape containment");
            return false;
        }
        return MoveTo(KARAZHAN_MAP_ID, safeEscape.GetPositionX(), safeEscape.GetPositionY(), safeEscape.GetPositionZ(),
                      false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    if (!hasSafeAnchor)
    {
        LogKarazhanNightbaneDebug(
            bot, "Nightbane: rejected unsafe anchor, holding current position."
            " reason=no safe dynamic anchor"
            " raw_anchor=(" + std::to_string(anchor.GetPositionX()) + "," + std::to_string(anchor.GetPositionY()) + "," +
            std::to_string(anchor.GetPositionZ()) + ")"
            " bot=(" + std::to_string(bot->GetPositionX()) + "," + std::to_string(bot->GetPositionY()) + "," +
            std::to_string(bot->GetPositionZ()) + ")"
            " boss=(" + std::to_string(nightbane->GetPositionX()) + "," + std::to_string(nightbane->GetPositionY()) + "," +
            std::to_string(nightbane->GetPositionZ()) + ") face=" + std::to_string(nightbane->GetOrientation()) +
            " direct=" + std::to_string(anchorDist) + " zdiff=" + std::to_string(zDiffRaw));
        return false;
    }

    const float safeDist = bot->GetExactDist2d(safeAnchor.GetPositionX(), safeAnchor.GetPositionY());
    if (!IsAtNightbaneDynamicAnchor(bot, safeAnchor, isRanged || isHealer ? 3.0f : 2.0f))
    {
        LogKarazhanNightbaneDebug(
            bot, "dynamic-ground mode=reanchor role=" + role +
            " boss=(" + std::to_string(nightbane->GetPositionX()) + "," + std::to_string(nightbane->GetPositionY()) + "," +
            std::to_string(nightbane->GetPositionZ()) + ") face=" + std::to_string(nightbane->GetOrientation()) +
            " anchor=(" + std::to_string(anchor.GetPositionX()) + "," + std::to_string(anchor.GetPositionY()) + "," +
            std::to_string(anchor.GetPositionZ()) + ") safe_anchor=(" + std::to_string(safeAnchor.GetPositionX()) + "," +
            std::to_string(safeAnchor.GetPositionY()) + "," + std::to_string(safeAnchor.GetPositionZ()) + ")" +
            " direct=" + std::to_string(anchorDist) + " safe_dist=" + std::to_string(safeDist) +
            " zdiff=" + std::to_string(zDiffRaw));
        if (!IsNightbaneMovementAllowed(bot, safeAnchor))
        {
            LogKarazhanNightbaneDebug(bot, "Nightbane: rejected unsafe anchor, holding current position. reason=reanchor containment");
            return false;
        }
        return MoveTo(KARAZHAN_MAP_ID, safeAnchor.GetPositionX(), safeAnchor.GetPositionY(), safeAnchor.GetPositionZ(),
                      false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
    }

    LogKarazhanNightbaneDebug(
        bot, "dynamic-ground mode=hold role=" + role +
        " anchor=(" + std::to_string(anchor.GetPositionX()) + "," + std::to_string(anchor.GetPositionY()) + "," +
        std::to_string(anchor.GetPositionZ()) + ") safe_anchor=(" + std::to_string(safeAnchor.GetPositionX()) + "," +
        std::to_string(safeAnchor.GetPositionY()) + "," + std::to_string(safeAnchor.GetPositionZ()) + ")" +
        " direct=" + std::to_string(anchorDist) + " safe_dist=" + std::to_string(safeDist) + " zdiff=" + std::to_string(zDiffRaw));
    return false;
}

// Ranged bots rotate between 3 positions to avoid standing in Charred Earth, which lasts for
// 30s and has a minimum cooldown of 18s (so there can be 2 active at once)
// Ranged positions are near the Northeastern door to the tower
bool NightbaneGroundPhaseRotateRangedPositionsAction::Execute(Event /*event*/)
{
    if (!IsKarazhanNightbaneEnabled())
        return false;

    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (nightbane && ShouldUseDynamicHumanTankMode(botAI, bot, nightbane))
    {
        LogKarazhanNightbaneDebug(bot, "ground-phase ranged fixed slots skipped: human-tank mode active");
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
            LogKarazhanNightbaneDebug(bot, "ground-phase: charred-earth immediate escape");
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
                LogKarazhanNightbaneDebug(bot, "Nightbane containment reject: ranged rotate destination invalid");
                return false;
            }
            LogKarazhanNightbaneDebug(bot, "ground-phase: rotating ranged charred-earth slot");
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
            LogKarazhanNightbaneDebug(bot, "Nightbane containment reject: ranged restore destination invalid");
            return false;
        }
        LogKarazhanNightbaneDebug(
            bot, "ground-phase: restoring ranged slot dist=" + std::to_string(distanceToTarget));
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
    if (!IsKarazhanNightbaneEnabled())
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
        LogKarazhanNightbaneDebug(bot, "casting fear ward on main tank");
        return botAI->CastSpell("fear ward", mainTank);
    }

    return false;
}

// Put pets on passive during the flight phase so they don't try to chase Nightbane off the map
bool NightbaneControlPetAggressionAction::Execute(Event /*event*/)
{
    if (!IsKarazhanNightbaneEnabled())
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
    if (!IsKarazhanNightbaneEnabled())
        return false;

    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (!nightbane || nightbane->GetPositionZ() <= NIGHTBANE_FLIGHT_Z)
    {
        LogKarazhanNightbaneDebug(bot, "flight-phase: inactive");
        return false;
    }

    LogKarazhanNightbaneDebug(bot, "flight-phase: preserving skull target, moon mark suppressed");

    Unit* botTarget = botAI->GetUnit(bot->GetTarget());
    if (botTarget && botTarget == nightbane)
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
            LogKarazhanNightbaneDebug(
                bot, "Nightbane containment reject: flight stack destination invalid"
                " dest=(" + std::to_string(destX) + "," + std::to_string(destY) + "," + std::to_string(destZ) + ")");
            return false;
        }
        LogKarazhanNightbaneDebug(
            bot, "flight-phase: moving to stack x=" + std::to_string(destX) +
            " y=" + std::to_string(destY) +
            " dist=" + std::to_string(bot->GetExactDist2d(destX, destY)));
        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        return MoveTo(KARAZHAN_MAP_ID, destX, destY, destZ, false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

bool NightbaneManageTimersAndTrackersAction::Execute(Event /*event*/)
{
    if (!IsKarazhanNightbaneEnabled())
        return false;

    Unit* nightbane = AI_VALUE2(Unit*, "find target", "nightbane");
    if (!nightbane)
    {
        LogKarazhanNightbaneDebug(bot, "timers: no target");
        return false;
    }

    const uint32 instanceId = nightbane->GetMap()->GetInstanceId();
    const ObjectGuid botGuid = bot->GetGUID();
    const time_t now = std::time(nullptr);
    const bool isGround = nightbane->GetPositionZ() <= NIGHTBANE_FLIGHT_Z;
    const bool wasInFlight = nightbaneWasInFlightPhase.count(instanceId) ? nightbaneWasInFlightPhase[instanceId] : false;

    if (!IsInsideNightbaneFightArea(Position(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ())))
    {
        LogKarazhanNightbaneDebug(
            bot, "Nightbane leash: bot outside safe area"
            " bot=(" + std::to_string(bot->GetPositionX()) + "," + std::to_string(bot->GetPositionY()) + "," +
            std::to_string(bot->GetPositionZ()) + ")");
        bot->AttackStop();
        if (MotionMaster* mm = bot->GetMotionMaster())
            mm->Clear(false);

        Position safe = GetNearestNightbaneSafePoint(bot);
        if (IsNightbaneMovementAllowed(bot, safe))
        {
            LogKarazhanNightbaneDebug(
                bot, "Nightbane leash decision=return-to-safe"
                " safe=(" + std::to_string(safe.GetPositionX()) + "," + std::to_string(safe.GetPositionY()) + "," +
                std::to_string(safe.GetPositionZ()) + ")");
            return MoveTo(KARAZHAN_MAP_ID, safe.GetPositionX(), safe.GetPositionY(), safe.GetPositionZ(),
                          false, false, false, false, MovementPriority::MOVEMENT_FORCED, true, false);
        }

        LogKarazhanNightbaneDebug(bot, "Nightbane leash decision=hold-no-safe-path");
        return false;
    }

    // Erase DPS wait timer and tank and ranged position tracking on encounter reset
    if (nightbane->GetHealth() == nightbane->GetMaxHealth())
    {
        LogKarazhanNightbaneDebug(bot, "timers: reset state");
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
        LogKarazhanNightbaneDebug(bot, std::string("timers: ") + (justLanded ? "landing/post-air-ground" : "ground phase"));
        nightbaneRainOfBonesHit.erase(botGuid);

        if (Group* group = bot->GetGroup())
        {
            if (group->GetTargetIcon(RtiTargetValue::moonIndex) == nightbane->GetGUID())
            {
                group->SetTargetIcon(RtiTargetValue::moonIndex, bot->GetGUID(), ObjectGuid::Empty);
                LogKarazhanNightbaneDebug(bot, "landing: cleared stale moon mark from Nightbane");
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
        LogKarazhanNightbaneDebug(bot, "timers: flight phase");
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
