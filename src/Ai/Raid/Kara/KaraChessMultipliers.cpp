/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "KaraMultipliers.h"
#include "KaraActions.h"
#include "KaraHelpers.h"
#include "KaraChessHelpers.h"
#include "AttackAction.h"
#include "ChooseTargetActions.h"
#include "DruidActions.h"
#include "FollowActions.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "MovementActions.h"
#include "Playerbots.h"
#include "PriestActions.h"
#include "EncounterHelpers.h"
#include "ReachTargetActions.h"
#include "RogueActions.h"
#include "ShamanActions.h"

using namespace KarazhanHelpers;
float KarazhanChessDisableCombatActionsMultiplier::GetValue(Action* action)
{
    if (!IsChessEncounterRelevant(botAI, bot))
        return 1.0f;

    if (!IsChessEventActive(botAI, bot))
        return 1.0f;

    Creature* charm = bot->GetCharm() ? bot->GetCharm()->ToCreature() : nullptr;
    bool activeController = (charm && IsChessPieceEntry(charm->GetEntry())) || GetAssignedChessPiece(bot) != nullptr;
    if (!activeController)
        return 1.0f;

    if (dynamic_cast<KarazhanChessPassiveHelperAction*>(action) ||
        dynamic_cast<KarazhanChessClaimPieceAction*>(action) ||
        dynamic_cast<KarazhanChessMovePieceAction*>(action) ||
        dynamic_cast<KarazhanChessMoveOutOfFireAction*>(action) ||
        dynamic_cast<KarazhanChessUseAbilityAction*>(action) ||
        dynamic_cast<KarazhanChessHealFriendlyAction*>(action) ||
        dynamic_cast<KarazhanChessAttackEnemyKingAction*>(action) ||
        dynamic_cast<KarazhanChessBlockEnemyPathAction*>(action) ||
        dynamic_cast<KarazhanChessReleaseOrReassignAction*>(action))
        return 1.0f;

    if (dynamic_cast<AttackAction*>(action) ||
        (dynamic_cast<CastSpellAction*>(action) && !dynamic_cast<CastHealingSpellAction*>(action)) ||
        dynamic_cast<ReachTargetAction*>(action) ||
        dynamic_cast<TankAssistAction*>(action) ||
        dynamic_cast<CombatFormationMoveAction*>(action) ||
        dynamic_cast<FollowAction*>(action))
        return 0.0f;

    return 1.0f;
}
