/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "KaraTriggers.h"
#include "KaraHelpers.h"
#include "KaraChessHelpers.h"
#include "KaraActions.h"
#include "Playerbots.h"
#include "EncounterHelpers.h"

using namespace KarazhanHelpers;

bool KarazhanChessEventActiveTrigger::IsActive()
{
    if (!IsChessEncounterRelevant(botAI, bot))
        return false;

    return IsChessEventActive(botAI, bot);
}

bool KarazhanChessPieceNeedsControllerTrigger::IsActive()
{
    if (!IsChessEncounterRelevant(botAI, bot) || !IsChessEventActive(botAI, bot))
        return false;

    Creature* charm = bot->GetCharm() ? bot->GetCharm()->ToCreature() : nullptr;
    if (charm && IsChessPieceEntry(charm->GetEntry()))
    {
        if (IsOnActiveChessBoard(charm))
            return false;

        return true;
    }

    Creature* assigned = GetAssignedChessPiece(bot);
    if (!assigned || assigned->IsCharmed())
        return true;

    return false;
}

bool KarazhanControlledChessPieceInFireTrigger::IsActive()
{
    if (!IsChessEncounterRelevant(botAI, bot) || !IsChessEventActive(botAI, bot))
        return false;

    Creature* piece = bot->GetCharm() ? bot->GetCharm()->ToCreature() : nullptr;
    if (!piece || !IsChessPieceEntry(piece->GetEntry()))
        return false;

    GuidVector const npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();
    for (ObjectGuid const& npcGuid : npcs)
    {
        Creature* unit = botAI->GetCreature(npcGuid);
        if (unit && unit->GetEntry() == NPC_CHESS_EVENT_MEDIVH_CHEAT_FIRES && unit->GetExactDist2d(piece) < 4.0f)
            return true;
    }

    return false;
}

bool KarazhanFriendlyKingUnderThreatTrigger::IsActive()
{
    if (!IsChessEncounterRelevant(botAI, bot) || !IsChessEventActive(botAI, bot))
        return false;

    Creature* king = GetFriendlyChessKing(botAI, bot);
    if (!king)
        return false;

    return king->GetMaxHealth() - king->GetHealth() > 5000;
}

bool KarazhanEnemyKingVulnerableTrigger::IsActive()
{
    if (!IsChessEncounterRelevant(botAI, bot) || !IsChessEventActive(botAI, bot))
        return false;

    Creature* enemyKing = GetEnemyChessKing(botAI, bot);
    if (!enemyKing)
        return false;

    Creature* piece = bot->GetCharm() ? bot->GetCharm()->ToCreature() : nullptr;
    if (!piece)
        return false;

    uint32 supportAlive = 0;
    uint32 damageAlive = 0;
    uint32 pawnAlive = 0;
    uint32 activeNonKingRemaining = 0;
    std::string gateReason;
    bool const attackAllowed = IsKarazhanChessKingFocusAllowedActiveBoard(botAI, bot, enemyKing, supportAlive, damageAlive, pawnAlive, activeNonKingRemaining, gateReason);

    if (!attackAllowed)
        return false;

    bool const kingCriticallyLow = gateReason == "king_critically_low";
    return piece->GetDistance(enemyKing) < 30.0f || kingCriticallyLow;
}

bool KarazhanControlledChessPieceAbilityReadyTrigger::IsActive()
{
    if (!IsChessEncounterRelevant(botAI, bot) || !IsChessEventActive(botAI, bot))
        return false;

    Creature* piece = bot->GetCharm() ? bot->GetCharm()->ToCreature() : nullptr;
    if (!piece || !IsChessPieceEntry(piece->GetEntry()))
        return false;

    for (uint8 i = 0; i < MAX_CREATURE_SPELLS; ++i)
    {
        uint32 spellId = piece->m_spells[i];
        if (spellId && !piece->HasSpellCooldown(spellId))
            return true;
    }

    return false;
}
