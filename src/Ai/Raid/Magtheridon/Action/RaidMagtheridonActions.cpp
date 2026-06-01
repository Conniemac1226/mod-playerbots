#include "RaidMagtheridonActions.h"
#include "RaidMagtheridonHelpers.h"
#include "Creature.h"
#include "ObjectAccessor.h"
#include "ObjectGuid.h"
#include "Playerbots.h"
#include "RaidBossHelpers.h"

using namespace MagtheridonHelpers;

namespace
{
    bool HasLivingHunterBotSupport(Group* group)
    {
        if (!group)
            return false;

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->IsAlive() && member->getClass() == CLASS_HUNTER &&
                GET_PLAYERBOT_AI(member))
                return true;
        }

        return false;
    }

    bool ShouldAttackPriorityTarget(Player* bot, Unit* target)
    {
        return bot && target && (bot->GetTarget() != target->GetGUID() || bot->GetVictim() != target);
    }
}

bool MagtheridonAutoPullTrashAction::Execute(Event event)
{
    if (!IsMagtheridonAutoPullReady(botAI, bot))
    {
        LogMagtheridonDebug(botAI, bot, "auto_pull_hold",
            "reason=not_ready " + GetMagtheridonEncounterDebug(botAI, bot, FindMagtheridon(botAI, bot)), nullptr, 10);
        return false;
    }

    Unit* target = SelectMagtheridonTrashPullTarget(botAI, bot);
    if (!target)
    {
        LogMagtheridonDebug(botAI, bot, "auto_pull_no_target",
            "reason=no_trash_candidate " + GetMagtheridonEncounterDebug(botAI, bot, FindMagtheridon(botAI, bot)), nullptr, 10);
        return false;
    }

    context->GetValue<Unit*>("current target")->Set(target);
    bot->SetSelection(target->GetGUID());
    LogMagtheridonDebug(botAI, bot, "auto_pull_target_selected",
        GetMagtheridonDebugUnit(bot, target), nullptr, 0);

    bool usedRangedOpener = false;
    if (botAI->IsTank(bot))
    {
        switch (bot->getClass())
        {
            case CLASS_PALADIN:
                usedRangedOpener = botAI->DoSpecificAction("avenger's shield", event, true) ||
                    botAI->DoSpecificAction("hand of reckoning", event, true);
                break;
            case CLASS_WARRIOR:
                usedRangedOpener = botAI->DoSpecificAction("heroic throw", event, true);
                break;
            case CLASS_DRUID:
                usedRangedOpener = botAI->DoSpecificAction("faerie fire (feral)", event, true) ||
                    botAI->DoSpecificAction("growl", event, true);
                break;
            case CLASS_DEATH_KNIGHT:
                usedRangedOpener = botAI->DoSpecificAction("death grip", event, true) ||
                    botAI->DoSpecificAction("icy touch", event, true) ||
                    botAI->DoSpecificAction("dark command", event, true);
                break;
            default:
                break;
        }
    }

    bool attackStarted = Attack(target);
    if (usedRangedOpener || attackStarted)
    {
        botAI->SetNextCheckDelay(sPlayerbotAIConfig.reactDelay);
        return true;
    }

    return false;
}

bool MagtheridonAutoPullTrashAction::isUseful()
{
    return IsMagtheridonAutoPullReady(botAI, bot) &&
           SelectMagtheridonTrashPullTarget(botAI, bot);
}

bool MagtheridonMainTankAttackFirstThreeChannelersAction::Execute(Event /*event*/)
{
    Unit* magtheridon = FindMagtheridon(botAI, bot);
    if (!magtheridon || !IsMagtheridonChannelerPhaseActive(botAI, bot))
    {
        LogMagtheridonDebug(botAI, bot, "mt_channeler_attack_hold",
            "reason=phase_inactive " + GetMagtheridonEncounterDebug(botAI, bot, magtheridon), magtheridon, 8);
        return false;
    }

    Creature* channelerSquare = GetChanneler(bot, SOUTH_CHANNELER);
    if (channelerSquare)
        MarkTargetWithSquare(bot, channelerSquare);

    Creature* channelerStar   = GetChanneler(bot, WEST_CHANNELER);
    if (channelerStar)
        MarkTargetWithStar(bot, channelerStar);

    Creature* channelerCircle = GetChanneler(bot, EAST_CHANNELER);
    if (channelerCircle)
        MarkTargetWithCircle(bot, channelerCircle);

    // After first three channelers are dead, wait for Magtheridon to activate
    if (!channelerSquare && !channelerStar && !channelerCircle)
    {
        const Position& position = WAITING_FOR_MAGTHERIDON_POSITION;
        if (!bot->IsWithinDist2d(position.GetPositionX(), position.GetPositionY(), 2.0f))
        {
            LogMagtheridonDebug(botAI, bot, "mt_channeler_wait_move",
                "reason=wait_for_magtheridon " + GetMagtheridonEncounterDebug(botAI, bot, magtheridon), magtheridon, 0);
            return MoveTo(MAGTHERIDON_MAP_ID, position.GetPositionX(), position.GetPositionY(),
                          position.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }

        bot->SetFacingTo(position.GetOrientation());
        LogMagtheridonDebug(botAI, bot, "mt_channeler_wait_hold",
            "reason=waiting_for_boss " + GetMagtheridonEncounterDebug(botAI, bot, magtheridon), magtheridon, 8);
        return true;
    }

    Creature* currentTarget = nullptr;
    std::string rtiName;
    if (channelerSquare)
    {
        currentTarget = channelerSquare;
        rtiName = "square";
    }
    else if (channelerStar)
    {
        currentTarget = channelerStar;
        rtiName = "star";
    }
    else if (channelerCircle)
    {
        currentTarget = channelerCircle;
        rtiName = "circle";
    }

    SetRtiTarget(botAI, rtiName, currentTarget);
    LogMagtheridonDebug(botAI, bot, "mt_channeler_target",
        "selected=" + GetMagtheridonDebugUnit(bot, currentTarget) + " rti=" + rtiName, magtheridon, 0);

    Group* group = bot->GetGroup();
    bool hasHunterSupport = HasLivingHunterBotSupport(group);
    if (currentTarget && !hasHunterSupport &&
        (currentTarget == channelerStar || currentTarget == channelerCircle))
    {
        float const directPickupRange = 9.0f;
        if (bot->GetDistance(currentTarget) > directPickupRange)
        {
            return MoveTo(MAGTHERIDON_MAP_ID,
                          currentTarget->GetPositionX(),
                          currentTarget->GetPositionY(),
                          currentTarget->GetPositionZ(),
                          false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    if (currentTarget && bot->GetVictim() != currentTarget)
    {
        LogMagtheridonDebug(botAI, bot, "mt_channeler_attack",
            "selected=" + GetMagtheridonDebugUnit(bot, currentTarget) + " rti=" + rtiName, magtheridon, 0);
        return Attack(currentTarget);
    }

    LogMagtheridonDebug(botAI, bot, "mt_channeler_hold",
        "reason=already_on_target_or_no_target rti=" + rtiName + " " + GetMagtheridonEncounterDebug(botAI, bot, magtheridon), magtheridon, 8);
    return false;
}

bool MagtheridonFirstAssistTankAttackNWChannelerAction::Execute(Event /*event*/)
{
    if (!IsMagtheridonChannelerPhaseActive(botAI, bot))
    {
        LogMagtheridonDebug(botAI, bot, "assist1_channeler_hold",
            "reason=phase_inactive", FindMagtheridon(botAI, bot), 8);
        return false;
    }

    Creature* channelerDiamond = GetChanneler(bot, NORTHWEST_CHANNELER);
    if (!channelerDiamond)
    {
        LogMagtheridonDebug(botAI, bot, "assist1_channeler_hold",
            "reason=no_channeler target=NW", FindMagtheridon(botAI, bot), 8);
        return false;
    }

    MarkTargetWithDiamond(bot, channelerDiamond);
    SetRtiTarget(botAI, "diamond", channelerDiamond);

    const Position& position = NW_CHANNELER_TANK_POSITION;
    const float maxDistance = 3.0f;
    float distanceToPosition =
        bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

    if (distanceToPosition > maxDistance)
    {
        float dX = position.GetPositionX() - bot->GetPositionX();
        float dY = position.GetPositionY() - bot->GetPositionY();
        float moveX = bot->GetPositionX() + (dX / distanceToPosition) * maxDistance;
        float moveY = bot->GetPositionY() + (dY / distanceToPosition) * maxDistance;

        LogMagtheridonDebug(botAI, bot, "assist1_channeler_move",
            "reason=positioning " + GetMagtheridonDebugUnit(bot, channelerDiamond), FindMagtheridon(botAI, bot), 0);
        return MoveTo(MAGTHERIDON_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    if (bot->GetVictim() != channelerDiamond)
    {
        LogMagtheridonDebug(botAI, bot, "assist1_channeler_attack",
            GetMagtheridonDebugUnit(bot, channelerDiamond), FindMagtheridon(botAI, bot), 0);
        return Attack(channelerDiamond);
    }

    LogMagtheridonDebug(botAI, bot, "assist1_channeler_hold",
        "reason=already_on_target " + GetMagtheridonDebugUnit(bot, channelerDiamond), FindMagtheridon(botAI, bot), 8);
    return false;
}

bool MagtheridonSecondAssistTankAttackNEChannelerAction::Execute(Event /*event*/)
{
    if (!IsMagtheridonChannelerPhaseActive(botAI, bot))
    {
        LogMagtheridonDebug(botAI, bot, "assist2_channeler_hold",
            "reason=phase_inactive", FindMagtheridon(botAI, bot), 8);
        return false;
    }

    Creature* channelerTriangle = GetChanneler(bot, NORTHEAST_CHANNELER);
    if (!channelerTriangle)
    {
        LogMagtheridonDebug(botAI, bot, "assist2_channeler_hold",
            "reason=no_channeler target=NE", FindMagtheridon(botAI, bot), 8);
        return false;
    }

    MarkTargetWithTriangle(bot, channelerTriangle);
    SetRtiTarget(botAI, "triangle", channelerTriangle);

    const Position& position = NE_CHANNELER_TANK_POSITION;
    const float maxDistance = 3.0f;
    float distanceToPosition =
        bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

    if (distanceToPosition > maxDistance)
    {
        float dX = position.GetPositionX() - bot->GetPositionX();
        float dY = position.GetPositionY() - bot->GetPositionY();
        float moveX = bot->GetPositionX() + (dX / distanceToPosition) * maxDistance;
        float moveY = bot->GetPositionY() + (dY / distanceToPosition) * maxDistance;

        LogMagtheridonDebug(botAI, bot, "assist2_channeler_move",
            "reason=positioning " + GetMagtheridonDebugUnit(bot, channelerTriangle), FindMagtheridon(botAI, bot), 0);
        return MoveTo(MAGTHERIDON_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    if (bot->GetVictim() != channelerTriangle)
    {
        LogMagtheridonDebug(botAI, bot, "assist2_channeler_attack",
            GetMagtheridonDebugUnit(bot, channelerTriangle), FindMagtheridon(botAI, bot), 0);
        return Attack(channelerTriangle);
    }

    LogMagtheridonDebug(botAI, bot, "assist2_channeler_hold",
        "reason=already_on_target " + GetMagtheridonDebugUnit(bot, channelerTriangle), FindMagtheridon(botAI, bot), 8);
    return false;
}

// Misdirect West & East Channelers to Main Tank
bool MagtheridonMisdirectHellfireChannelers::Execute(Event /*event*/)
{
    if (!IsMagtheridonChannelerPhaseActive(botAI, bot))
    {
        LogMagtheridonDebug(botAI, bot, "misdirect_hold",
            "reason=phase_inactive", FindMagtheridon(botAI, bot), 8);
        return false;
    }

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    std::vector<Player*> hunters;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->getClass() == CLASS_HUNTER && GET_PLAYERBOT_AI(member))
            hunters.push_back(member);
    }

    int hunterIndex = -1;
    for (size_t i = 0; i < hunters.size(); ++i)
    {
        if (hunters[i] == bot)
        {
            hunterIndex = static_cast<int>(i);
            break;
        }
    }

    Player* mainTank = nullptr;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && botAI->IsMainTank(member))
        {
            mainTank = member;
            break;
        }
    }

    Creature* channelerStar = GetChanneler(bot, WEST_CHANNELER);
    Creature* channelerCircle = GetChanneler(bot, EAST_CHANNELER);

    switch (hunterIndex)
    {
    case 0:
        if (mainTank && channelerStar &&
            channelerStar->GetVictim() != mainTank)
        {
            if (botAI->CanCastSpell("misdirection", mainTank))
            {
                LogMagtheridonDebug(botAI, bot, "misdirect_cast",
                    "target=" + GetMagtheridonDebugUnit(bot, mainTank) + " channeler=" + GetMagtheridonDebugUnit(bot, channelerStar),
                    FindMagtheridon(botAI, bot), 0);
                return botAI->CastSpell("misdirection", mainTank);
            }

            if (!bot->HasAura(SPELL_MISDIRECTION))
            {
                LogMagtheridonDebug(botAI, bot, "misdirect_hold",
                    "reason=no_aura_after_attempt channeler=" + GetMagtheridonDebugUnit(bot, channelerStar), FindMagtheridon(botAI, bot), 8);
                return false;
            }

            if (botAI->CanCastSpell("steady shot", channelerStar))
            {
                LogMagtheridonDebug(botAI, bot, "misdirect_cast",
                    "target=" + GetMagtheridonDebugUnit(bot, channelerStar), FindMagtheridon(botAI, bot), 0);
                return botAI->CastSpell("steady shot", channelerStar);
            }
        }
        break;

    case 1:
        if (mainTank && channelerCircle &&
            channelerCircle->GetVictim() != mainTank)
        {
            if (botAI->CanCastSpell("misdirection", mainTank))
            {
                LogMagtheridonDebug(botAI, bot, "misdirect_cast",
                    "target=" + GetMagtheridonDebugUnit(bot, mainTank) + " channeler=" + GetMagtheridonDebugUnit(bot, channelerCircle),
                    FindMagtheridon(botAI, bot), 0);
                return botAI->CastSpell("misdirection", mainTank);
            }

            if (!bot->HasAura(SPELL_MISDIRECTION))
            {
                LogMagtheridonDebug(botAI, bot, "misdirect_hold",
                    "reason=no_aura_after_attempt channeler=" + GetMagtheridonDebugUnit(bot, channelerCircle), FindMagtheridon(botAI, bot), 8);
                return false;
            }

            if (botAI->CanCastSpell("steady shot", channelerCircle))
            {
                LogMagtheridonDebug(botAI, bot, "misdirect_cast",
                    "target=" + GetMagtheridonDebugUnit(bot, channelerCircle), FindMagtheridon(botAI, bot), 0);
                return botAI->CastSpell("steady shot", channelerCircle);
            }
        }
        break;

    default:
        break;
    }

    LogMagtheridonDebug(botAI, bot, "misdirect_hold",
        "reason=no_valid_channeler_or_main_tank", FindMagtheridon(botAI, bot), 8);
    return false;
}

bool MagtheridonAssignDPSPriorityAction::Execute(Event /*event*/)
{
    Unit* previousTarget = botAI ? botAI->GetUnit(bot->GetTarget()) : nullptr;

    if (!IsMagtheridonChannelerPhaseActive(botAI, bot))
    {
        LogMagtheridonDebug(botAI, bot, "dps_priority_hold",
            "reason=phase_inactive " + GetMagtheridonTargetDecisionFields(previousTarget, nullptr, nullptr, "dps_priority_hold", "dps_priority_attack"),
            FindMagtheridon(botAI, bot), 8);
        return false;
    }

    Group* group = bot->GetGroup();
    bool hasHunterSupport = HasLivingHunterBotSupport(group);
    Unit* magtheridon = FindMagtheridon(botAI, bot);
    bool magtheridonActive = magtheridon && !magtheridon->HasAura(SPELL_SHADOW_CAGE);

    // Listed in order of priority
    Creature* channelerSquare   = GetChanneler(bot, SOUTH_CHANNELER);
    if (channelerSquare)
    {
        SetRtiTarget(botAI, "square", channelerSquare);

        if (bot->GetTarget() != channelerSquare->GetGUID())
        {
            LogMagtheridonDebug(botAI, bot, "dps_priority_attack",
                "priority=square " + GetMagtheridonTargetDecisionFields(previousTarget, channelerSquare, nullptr, "dps_priority_attack", "none"),
                FindMagtheridon(botAI, bot), 0);
            return Attack(channelerSquare);
        }

        if (ShouldAttackPriorityTarget(bot, channelerSquare))
        {
            LogMagtheridonDebug(botAI, bot, "dps_priority_attack",
                "priority=square reason=target_selected_not_engaged " +
                GetMagtheridonTargetDecisionFields(previousTarget, channelerSquare, nullptr, "dps_priority_attack", "dps_priority_hold"),
                magtheridon, 0);
            return Attack(channelerSquare);
        }

        LogMagtheridonDebug(botAI, bot, "dps_priority_hold",
            "priority=square reason=already_on_target " + GetMagtheridonTargetDecisionFields(previousTarget, channelerSquare, nullptr, "dps_priority_hold", "dps_priority_attack"),
            FindMagtheridon(botAI, bot), 8);
        return false;
    }

    Creature* channelerStar = GetChanneler(bot, WEST_CHANNELER);
    if (channelerStar)
    {
        if (!magtheridonActive && !hasHunterSupport && !channelerStar->IsInCombat())
        {
            LogMagtheridonDebug(botAI, bot, "dps_priority_hold",
                "priority=star reason=waiting_for_hunter_support " + GetMagtheridonDebugUnit(bot, channelerStar), FindMagtheridon(botAI, bot), 8);
            return false;
        }

        SetRtiTarget(botAI, "star", channelerStar);

        if (bot->GetTarget() != channelerStar->GetGUID())
        {
            LogMagtheridonDebug(botAI, bot, "dps_priority_attack",
                "priority=star " + GetMagtheridonTargetDecisionFields(previousTarget, channelerStar, nullptr, "dps_priority_attack", "none"),
                FindMagtheridon(botAI, bot), 0);
            return Attack(channelerStar);
        }

        if (ShouldAttackPriorityTarget(bot, channelerStar))
        {
            LogMagtheridonDebug(botAI, bot, "dps_priority_attack",
                "priority=star reason=target_selected_not_engaged " +
                GetMagtheridonTargetDecisionFields(previousTarget, channelerStar, nullptr, "dps_priority_attack", "dps_priority_hold"),
                magtheridon, 0);
            return Attack(channelerStar);
        }

        LogMagtheridonDebug(botAI, bot, "dps_priority_hold",
            "priority=star reason=already_on_target " + GetMagtheridonTargetDecisionFields(previousTarget, channelerStar, nullptr, "dps_priority_hold", "dps_priority_attack"),
            FindMagtheridon(botAI, bot), 8);
        return false;
    }

    Creature* channelerCircle = GetChanneler(bot, EAST_CHANNELER);
    if (channelerCircle)
    {
        if (!magtheridonActive && !hasHunterSupport && !channelerCircle->IsInCombat())
        {
            LogMagtheridonDebug(botAI, bot, "dps_priority_hold",
                "priority=circle reason=waiting_for_hunter_support " + GetMagtheridonDebugUnit(bot, channelerCircle), FindMagtheridon(botAI, bot), 8);
            return false;
        }

        SetRtiTarget(botAI, "circle", channelerCircle);

        if (bot->GetTarget() != channelerCircle->GetGUID())
        {
            LogMagtheridonDebug(botAI, bot, "dps_priority_attack",
                "priority=circle " + GetMagtheridonTargetDecisionFields(previousTarget, channelerCircle, nullptr, "dps_priority_attack", "none"),
                FindMagtheridon(botAI, bot), 0);
            return Attack(channelerCircle);
        }

        if (ShouldAttackPriorityTarget(bot, channelerCircle))
        {
            LogMagtheridonDebug(botAI, bot, "dps_priority_attack",
                "priority=circle reason=target_selected_not_engaged " +
                GetMagtheridonTargetDecisionFields(previousTarget, channelerCircle, nullptr, "dps_priority_attack", "dps_priority_hold"),
                magtheridon, 0);
            return Attack(channelerCircle);
        }

        LogMagtheridonDebug(botAI, bot, "dps_priority_hold",
            "priority=circle reason=already_on_target " + GetMagtheridonTargetDecisionFields(previousTarget, channelerCircle, nullptr, "dps_priority_hold", "dps_priority_attack"),
            FindMagtheridon(botAI, bot), 8);
        return false;
    }

    Creature* channelerDiamond  = GetChanneler(bot, NORTHWEST_CHANNELER);
    if (channelerDiamond)
    {
        SetRtiTarget(botAI, "diamond", channelerDiamond);

        if (bot->GetTarget() != channelerDiamond->GetGUID())
        {
            LogMagtheridonDebug(botAI, bot, "dps_priority_attack",
                "priority=diamond " + GetMagtheridonTargetDecisionFields(previousTarget, channelerDiamond, nullptr, "dps_priority_attack", "none"),
                FindMagtheridon(botAI, bot), 0);
            return Attack(channelerDiamond);
        }

        if (ShouldAttackPriorityTarget(bot, channelerDiamond))
        {
            LogMagtheridonDebug(botAI, bot, "dps_priority_attack",
                "priority=diamond reason=target_selected_not_engaged " +
                GetMagtheridonTargetDecisionFields(previousTarget, channelerDiamond, nullptr, "dps_priority_attack", "dps_priority_hold"),
                magtheridon, 0);
            return Attack(channelerDiamond);
        }

        LogMagtheridonDebug(botAI, bot, "dps_priority_hold",
            "priority=diamond reason=already_on_target " + GetMagtheridonTargetDecisionFields(previousTarget, channelerDiamond, nullptr, "dps_priority_hold", "dps_priority_attack"),
            FindMagtheridon(botAI, bot), 8);
        return false;
    }

    Creature* channelerTriangle = GetChanneler(bot, NORTHEAST_CHANNELER);
    if (channelerTriangle)
    {
        SetRtiTarget(botAI, "triangle", channelerTriangle);

        if (bot->GetTarget() != channelerTriangle->GetGUID())
        {
            LogMagtheridonDebug(botAI, bot, "dps_priority_attack",
                "priority=triangle " + GetMagtheridonTargetDecisionFields(previousTarget, channelerTriangle, nullptr, "dps_priority_attack", "none"),
                FindMagtheridon(botAI, bot), 0);
            return Attack(channelerTriangle);
        }

        if (ShouldAttackPriorityTarget(bot, channelerTriangle))
        {
            LogMagtheridonDebug(botAI, bot, "dps_priority_attack",
                "priority=triangle reason=target_selected_not_engaged " +
                GetMagtheridonTargetDecisionFields(previousTarget, channelerTriangle, nullptr, "dps_priority_attack", "dps_priority_hold"),
                magtheridon, 0);
            return Attack(channelerTriangle);
        }

        LogMagtheridonDebug(botAI, bot, "dps_priority_hold",
            "priority=triangle reason=already_on_target " + GetMagtheridonTargetDecisionFields(previousTarget, channelerTriangle, nullptr, "dps_priority_hold", "dps_priority_attack"),
            FindMagtheridon(botAI, bot), 8);
        return false;
    }

    if (magtheridon && !magtheridon->HasAura(SPELL_SHADOW_CAGE) &&
        !channelerSquare && !channelerStar && !channelerCircle &&
        !channelerDiamond && !channelerTriangle)
    {
        SetRtiTarget(botAI, "cross", magtheridon);

        if (bot->GetTarget() != magtheridon->GetGUID())
        {
            LogMagtheridonDebug(botAI, bot, "dps_priority_attack",
                "priority=cross " + GetMagtheridonTargetDecisionFields(previousTarget, magtheridon, nullptr, "dps_priority_attack", "none"),
                magtheridon, 0);
            return Attack(magtheridon);
        }

        if (ShouldAttackPriorityTarget(bot, magtheridon))
        {
            LogMagtheridonDebug(botAI, bot, "dps_priority_attack",
                "priority=cross reason=target_selected_not_engaged " +
                GetMagtheridonTargetDecisionFields(previousTarget, magtheridon, nullptr, "dps_priority_attack", "dps_priority_hold"),
                magtheridon, 0);
            return Attack(magtheridon);
        }
    }

    std::string holdReason = "reason=no_selected_target";
    if (magtheridon)
    {
        if (magtheridon->HasAura(SPELL_SHADOW_CAGE))
            holdReason = "reason=shadow_cage_active";
        else if (bot->GetTarget() == magtheridon->GetGUID())
            holdReason = "reason=already_on_magtheridon";
        else if (channelerSquare || channelerStar || channelerCircle || channelerDiamond || channelerTriangle)
            holdReason = "reason=channelers_alive";
    }

    LogMagtheridonDebug(botAI, bot, "dps_priority_hold",
        holdReason + " " + GetMagtheridonTargetDecisionFields(previousTarget, botAI ? botAI->GetUnit(bot->GetTarget()) : nullptr, magtheridon, "dps_priority_hold", "dps_priority_attack") + " " + GetMagtheridonEncounterDebug(botAI, bot, magtheridon), magtheridon, 8);
    return false;
}

// Assign Burning Abyssals to Warlocks to Banish
// Burning Abyssals in excess of Warlocks in party will be Feared
bool MagtheridonWarlockCCBurningAbyssalAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
    {
        LogMagtheridonDebug(botAI, bot, "abyssal_cc_hold",
            "reason=no_group", FindMagtheridon(botAI, bot), 8);
        return false;
    }

    const GuidVector& npcs = AI_VALUE(GuidVector, "nearest hostile npcs");

    std::vector<Unit*> abyssals;
    for (auto const& npc : npcs)
    {
        Unit* unit = botAI->GetUnit(npc);
        if (unit && unit->GetEntry() == NPC_BURNING_ABYSSAL && unit->IsAlive())
            abyssals.push_back(unit);
    }

    std::vector<Player*> warlocks;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive() && member->getClass() == CLASS_WARLOCK && GET_PLAYERBOT_AI(member))
            warlocks.push_back(member);
    }

    int warlockIndex = -1;
    for (size_t i = 0; i < warlocks.size(); ++i)
    {
        if (warlocks[i] == bot)
        {
            warlockIndex = static_cast<int>(i);
            break;
        }
    }

    if (warlockIndex >= 0 && warlockIndex < abyssals.size())
    {
        Unit* assignedAbyssal = abyssals[warlockIndex];
        if (!botAI->HasAura("banish", assignedAbyssal) && botAI->CanCastSpell("banish", assignedAbyssal))
        {
            LogMagtheridonDebug(botAI, bot, "abyssal_cc_cast",
                "spell=banish assigned=" + GetMagtheridonDebugUnit(bot, assignedAbyssal), FindMagtheridon(botAI, bot), 0);
            return botAI->CastSpell("banish", assignedAbyssal);
        }

        LogMagtheridonDebug(botAI, bot, "abyssal_cc_hold",
            "reason=assigned_abyssal_unavailable assigned=" + GetMagtheridonDebugUnit(bot, assignedAbyssal), FindMagtheridon(botAI, bot), 8);
    }

    for (size_t i = warlocks.size(); i < abyssals.size(); ++i)
    {
        Unit* excessAbyssal = abyssals[i];
        if (!botAI->HasAura("banish", excessAbyssal) && !botAI->HasAura("fear", excessAbyssal) &&
            botAI->CanCastSpell("fear", excessAbyssal))
        {
            LogMagtheridonDebug(botAI, bot, "abyssal_cc_cast",
                "spell=fear assigned=" + GetMagtheridonDebugUnit(bot, excessAbyssal), FindMagtheridon(botAI, bot), 0);
            return botAI->CastSpell("fear", excessAbyssal);
        }
    }

    LogMagtheridonDebug(botAI, bot, "abyssal_cc_hold",
        "reason=no_castable_abyssal count=" + std::to_string(abyssals.size()), FindMagtheridon(botAI, bot), 8);
    return false;
}

// Main tank will back up to the Northern point of the room
bool MagtheridonMainTankPositionBossAction::Execute(Event /*event*/)
{
    Unit* magtheridon = FindMagtheridon(botAI, bot);
    Unit* previousTarget = botAI ? botAI->GetUnit(bot->GetTarget()) : nullptr;
    if (!magtheridon)
    {
        LogMagtheridonDebug(botAI, bot, "mt_boss_hold",
            "reason=no_magtheridon", nullptr, 8);
        return false;
    }

    MarkTargetWithCross(bot, magtheridon);
    SetRtiTarget(botAI, "cross", magtheridon);

    if (bot->GetVictim() != magtheridon)
    {
        LogMagtheridonDebug(botAI, bot, "mt_boss_attack",
            "selected=" + GetMagtheridonDebugUnit(bot, magtheridon) + " " +
            GetMagtheridonTargetDecisionFields(previousTarget, magtheridon, nullptr, "mt_boss_attack", "none"),
            magtheridon, 0);
        return Attack(magtheridon);
    }

    if (magtheridon->GetVictim() == bot)
    {
        const Position& position = MAGTHERIDON_TANK_POSITION;
        const float maxDistance = 2.0f;
        float distanceToPosition = bot->GetExactDist2d(position.GetPositionX(), position.GetPositionY());

        if (distanceToPosition > maxDistance)
        {
            float dX = position.GetPositionX() - bot->GetPositionX();
            float dY = position.GetPositionY() - bot->GetPositionY();
            float moveX = bot->GetPositionX() + (dX / distanceToPosition) * maxDistance;
            float moveY = bot->GetPositionY() + (dY / distanceToPosition) * maxDistance;

            LogMagtheridonDebug(botAI, bot, "mt_boss_move",
                "reason=hold_position " + GetMagtheridonTargetDecisionFields(previousTarget, magtheridon, nullptr, "mt_boss_move", "mt_boss_attack"),
                magtheridon, 0);
            return MoveTo(MAGTHERIDON_MAP_ID, moveX, moveY, position.GetPositionZ(), false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, true);
        }
    }

    LogMagtheridonDebug(botAI, bot, "mt_boss_hold",
        "reason=already_positioned " + GetMagtheridonTargetDecisionFields(previousTarget, magtheridon, nullptr, "mt_boss_hold", "mt_boss_move"),
        magtheridon, 8);
    return false;
}

// Ranged DPS will remain within 25 yards of the center of the room
// Healers will remain within 15 yards of a position that is between ranged DPS and the boss
std::unordered_map<ObjectGuid, Position> MagtheridonSpreadRangedAction::initialPositions;
std::unordered_map<ObjectGuid, bool> MagtheridonSpreadRangedAction::hasReachedInitialPosition;

bool MagtheridonSpreadRangedAction::Execute(Event /*event*/)
{
    Unit* magtheridon = FindMagtheridon(botAI, bot);
    Unit* previousTarget = botAI ? botAI->GetUnit(bot->GetTarget()) : nullptr;
    if (!magtheridon)
    {
        LogMagtheridonDebug(botAI, bot, "spread_hold",
            "reason=no_magtheridon", nullptr, 8);
        return false;
    }

    Group* group = bot->GetGroup();
    if (!group)
    {
        LogMagtheridonDebug(botAI, bot, "spread_hold",
            "reason=no_group", magtheridon, 8);
        return false;
    }

    const uint32 instanceId = magtheridon->GetMap()->GetInstanceId();

    // Wait for 6 seconds after Magtheridon activates to spread
    const uint8 spreadWaitSeconds = 6;
    auto it = spreadWaitTimer.find(instanceId);
    if (it == spreadWaitTimer.end() ||
        (time(nullptr) - it->second) < spreadWaitSeconds)
        return false;

    auto cubeIt = botToCubeAssignment.find(bot->GetGUID());
    if (cubeIt != botToCubeAssignment.end())
    {
        time_t now = time(nullptr);
        auto timerIt = blastNovaTimer.find(instanceId);
        if (timerIt != blastNovaTimer.end())
        {
            time_t lastBlastNova = timerIt->second;
            if (now - lastBlastNova >= 49)
                return false;
        }
    }

    std::vector<Player*> members;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && member->IsAlive())
            members.push_back(member);
    }

    bool isHealer = botAI->IsHeal(bot);
    const Position& center = isHealer
        ? HEALER_SPREAD_POSITION
        : RANGED_SPREAD_POSITION;
    float maxSpreadRadius = isHealer ? 15.0f : 20.0f;
    float centerX = center.GetPositionX();
    float centerY = center.GetPositionY();
    float centerZ = center.GetPositionZ();
    const float radiusBuffer = 3.0f;

    if (isHealer)
    {
        Player* lowestMember = nullptr;
        float lowestHp = 101.0f;
        auto urgentHealingNeeded = [&]() -> bool
        {
            for (Player* member : members)
            {
                if (!member || !member->IsAlive())
                    continue;

                if (member->GetHealthPct() < lowestHp)
                {
                    lowestHp = member->GetHealthPct();
                    lowestMember = member;
                }

                if (member->GetHealthPct() < 85.0f)
                    return true;
            }

            return false;
        };

        if (urgentHealingNeeded())
        {
            LogMagtheridonDebug(botAI, bot, "spread_healer_hold",
                "reason=urgent_healing_needed " +
                GetMagtheridonTargetDecisionFields(previousTarget, lowestMember, magtheridon, "spread_healer_hold", "heal_cast") +
                " " + GetMagtheridonDebugUnit(bot, lowestMember), magtheridon, 0);
            return false;
        }
    }

    if (!initialPositions.count(bot->GetGUID()))
    {
        auto it = std::find(members.begin(), members.end(), bot);
        uint8 botIndex = (it != members.end()) ? std::distance(members.begin(), it) : 0;
        uint8 count = members.size();

        float angle = 2 * M_PI * botIndex / count;
        float radius = static_cast<float>(rand()) / RAND_MAX * maxSpreadRadius;
        float targetX = centerX + radius * cos(angle);
        float targetY = centerY + radius * sin(angle);

        initialPositions[bot->GetGUID()] = Position(targetX, targetY, centerZ);
        hasReachedInitialPosition[bot->GetGUID()] = false;
    }

    Position targetPosition = initialPositions[bot->GetGUID()];
    if (!hasReachedInitialPosition[bot->GetGUID()])
    {
        if (!bot->IsWithinDist2d(targetPosition.GetPositionX(), targetPosition.GetPositionY(), 2.0f))
        {
            float destX = targetPosition.GetPositionX();
            float destY = targetPosition.GetPositionY();
            float destZ = targetPosition.GetPositionZ();

            if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(),
                bot->GetPositionY(), bot->GetPositionZ(), destX, destY, destZ))
                return false;

            if (!isHealer)
            {
                bot->AttackStop();
                bot->InterruptNonMeleeSpells(false);
            }
            LogMagtheridonDebug(botAI, bot, "spread_move",
                "reason=initial_position " + GetMagtheridonTargetDecisionFields(previousTarget, magtheridon, nullptr, "spread_move", "dps_priority_attack"),
                magtheridon, 0);
            return MoveTo(MAGTHERIDON_MAP_ID, destX, destY, destZ, false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
        hasReachedInitialPosition[bot->GetGUID()] = true;
    }

    float distToCenter = bot->GetExactDist2d(centerX, centerY);

    if (distToCenter > maxSpreadRadius + radiusBuffer)
    {
        float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * M_PI;
        float radius = static_cast<float>(rand()) / RAND_MAX * maxSpreadRadius;
        float targetX = centerX + radius * cos(angle);
        float targetY = centerY + radius * sin(angle);

        if (bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
            bot->GetPositionZ(), targetX, targetY, centerZ))
        {
            if (!isHealer)
            {
                bot->AttackStop();
                bot->InterruptNonMeleeSpells(false);
            }
            LogMagtheridonDebug(botAI, bot, "spread_move",
                "reason=too_far_from_center " + GetMagtheridonTargetDecisionFields(previousTarget, magtheridon, nullptr, "spread_move", "dps_priority_attack"),
                magtheridon, 0);
            return MoveTo(MAGTHERIDON_MAP_ID, targetX, targetY, centerZ, false, false, false, false,
                          MovementPriority::MOVEMENT_COMBAT, true, false);
        }
    }

    LogMagtheridonDebug(botAI, bot, "spread_hold",
        "reason=position_ok dist_to_center=" + std::to_string(distToCenter) + " " +
        GetMagtheridonTargetDecisionFields(previousTarget, magtheridon, nullptr, "spread_hold", "dps_priority_attack"),
        magtheridon, 8);
    return false;
}

// For bots that are assigned to click cubes
// Magtheridon casts Blast Nova every 54.35 to 55.40s, with a 2s cast time
bool MagtheridonUseManticronCubeAction::Execute(Event /*event*/)
{
    Unit* magtheridon = FindMagtheridon(botAI, bot);
    Unit* previousTarget = botAI ? botAI->GetUnit(bot->GetTarget()) : nullptr;
    if (!magtheridon)
    {
        LogMagtheridonDebug(botAI, bot, "cube_hold",
            "reason=no_magtheridon", nullptr, 8);
        return false;
    }

    auto it = botToCubeAssignment.find(bot->GetGUID());
    if (it == botToCubeAssignment.end())
    {
        LogMagtheridonDebug(botAI, bot, "cube_hold",
            "reason=no_cube_assignment " + GetMagtheridonTargetDecisionFields(previousTarget, magtheridon, nullptr, "cube_hold", "dps_priority_attack"),
            magtheridon, 8);
        return false;
    }

    const CubeInfo& cubeInfo = it->second;
    GameObject* cube = botAI->GetGameObject(cubeInfo.guid);
    if (!cube)
    {
        LogMagtheridonDebug(botAI, bot, "cube_hold",
            "reason=cube_missing " + GetMagtheridonTargetDecisionFields(previousTarget, magtheridon, nullptr, "cube_hold", "dps_priority_attack"),
            magtheridon, 8);
        return false;
    }

    // Release cubes after Blast Nova is interrupted
    if (HandleCubeRelease(magtheridon))
        return true;

    // Check if cube logic should be active (49+ second rule)
    if (!ShouldActivateCubeLogic(magtheridon))
    {
        LogMagtheridonDebug(botAI, bot, "cube_hold",
            "reason=blast_nova_timer_inactive assigned_cube_guid=" + cubeInfo.guid.ToString() + " " +
            GetMagtheridonTargetDecisionFields(previousTarget, magtheridon, nullptr, "cube_hold", "dps_priority_attack"),
            magtheridon, 8);
        return false;
    }

    // Handle active cube logic based on Blast Nova casting state
    bool blastNovaActive = magtheridon->HasUnitState(UNIT_STATE_CASTING) &&
                           magtheridon->FindCurrentSpellBySpellId(SPELL_BLAST_NOVA);

    if (!blastNovaActive)
        // After 49 seconds, wait at safe distance from cube
    {
        LogMagtheridonDebug(botAI, bot, "cube_wait",
            "reason=blast_nova_not_active assigned_cube_guid=" + cubeInfo.guid.ToString() +
            " cube_dist=" + std::to_string(bot->GetExactDist2d(cubeInfo.x, cubeInfo.y)) + " " +
            GetMagtheridonTargetDecisionFields(previousTarget, magtheridon, nullptr, "cube_wait", "dps_priority_attack"),
            magtheridon, 0);
        return HandleWaitingPhase(cubeInfo);
    }
    else
        // Blast Nova is casting - move to and click cube
    {
        LogMagtheridonDebug(botAI, bot, "cube_interact",
            "reason=blast_nova_active assigned_cube_guid=" + cubeInfo.guid.ToString() +
            " cube_dist=" + std::to_string(bot->GetExactDist2d(cubeInfo.x, cubeInfo.y)) + " " +
            GetMagtheridonTargetDecisionFields(previousTarget, magtheridon, nullptr, "cube_interact", "dps_priority_attack"),
            magtheridon, 0);
        return HandleCubeInteraction(cubeInfo, cube);
    }

    return false;
}

bool MagtheridonUseManticronCubeAction::HandleCubeRelease(Unit* magtheridon)
{
    if (bot->HasAura(SPELL_SHADOW_GRASP) &&
        !(magtheridon->HasUnitState(UNIT_STATE_CASTING) &&
          magtheridon->FindCurrentSpellBySpellId(SPELL_BLAST_NOVA)))
    {
        LogMagtheridonDebug(botAI, bot, "cube_release",
            "reason=blast_nova_interrupted", magtheridon, 0);
        uint32 delay = urand(200, 3000);
        botAI->AddTimedEvent(
            [this]
            {
                botAI->Reset();
            },
            delay);
        botAI->SetNextCheckDelay(delay + 50);
        return true;
    }

    return false;
}

bool MagtheridonUseManticronCubeAction::ShouldActivateCubeLogic(Unit* magtheridon)
{
    auto timerIt = blastNovaTimer.find(magtheridon->GetMap()->GetInstanceId());
    if (timerIt == blastNovaTimer.end())
        return false;

    time_t now = time(nullptr);
    time_t lastBlastNova = timerIt->second;

    return (now - lastBlastNova >= 49);
}

bool MagtheridonUseManticronCubeAction::HandleWaitingPhase(const CubeInfo& cubeInfo)
{
    const float safeWaitDistance = 8.0f;
    float cubeDist = bot->GetExactDist2d(cubeInfo.x, cubeInfo.y);

    if (fabs(cubeDist - safeWaitDistance) > 1.0f)
    {
        for (int i = 0; i < 12; ++i)
        {
            float angle = i * M_PI / 6.0f;
            float targetX = cubeInfo.x + cos(angle) * safeWaitDistance;
            float targetY = cubeInfo.y + sin(angle) * safeWaitDistance;
            float targetZ = bot->GetPositionZ();

            if (IsSafeFromMagtheridonHazards(botAI, bot, targetX, targetY, targetZ))
            {
                bot->AttackStop();
                bot->InterruptNonMeleeSpells(true);
                LogMagtheridonDebug(botAI, bot, "cube_wait_move",
                    "cube_guid=" + cubeInfo.guid.ToString() + " target_dist=" + std::to_string(cubeDist), FindMagtheridon(botAI, bot), 0);
                return MoveTo(MAGTHERIDON_MAP_ID, targetX, targetY, targetZ, false, false, false, false,
                              MovementPriority::MOVEMENT_COMBAT, true, false);
            }
        }

        float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * M_PI;
        float fallbackX = cubeInfo.x + cos(angle) * safeWaitDistance;
        float fallbackY = cubeInfo.y + sin(angle) * safeWaitDistance;
        float fallbackZ = bot->GetPositionZ();

        LogMagtheridonDebug(botAI, bot, "cube_wait_move",
            "reason=fallback cube_guid=" + cubeInfo.guid.ToString(), FindMagtheridon(botAI, bot), 0);
        return MoveTo(MAGTHERIDON_MAP_ID, fallbackX, fallbackY, fallbackZ, false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return true;
}

bool MagtheridonUseManticronCubeAction::HandleCubeInteraction(const CubeInfo& cubeInfo, GameObject* cube)
{
    const float interactDistance = 1.0f;
    float cubeDist = bot->GetExactDist2d(cubeInfo.x, cubeInfo.y);

    if (cubeDist > interactDistance)
    {
        if (cubeDist <= interactDistance + 1.0f)
        {
            uint32 delay = urand(200, 1500);
            botAI->AddTimedEvent(
                [this, cube]
                {
                    bot->StopMoving();
                    cube->Use(bot);
                },
                delay);
            botAI->SetNextCheckDelay(delay + 50);
            LogMagtheridonDebug(botAI, bot, "cube_click_scheduled",
                "cube_guid=" + cubeInfo.guid.ToString() + " dist=" + std::to_string(cubeDist), FindMagtheridon(botAI, bot), 0);
            return true;
        }

        float angle = atan2(cubeInfo.y - bot->GetPositionY(), cubeInfo.x - bot->GetPositionX());
        float targetX = cubeInfo.x - cos(angle) * interactDistance;
        float targetY = cubeInfo.y - sin(angle) * interactDistance;
        float targetZ = bot->GetPositionZ();

        bot->AttackStop();
        bot->InterruptNonMeleeSpells(true);
        LogMagtheridonDebug(botAI, bot, "cube_move_to_click",
            "cube_guid=" + cubeInfo.guid.ToString() + " dist=" + std::to_string(cubeDist), FindMagtheridon(botAI, bot), 0);
        return MoveTo(MAGTHERIDON_MAP_ID, targetX, targetY, targetZ, false, false, false, false,
                      MovementPriority::MOVEMENT_FORCED, true, false);
    }

    return false;
}

// The Blast Nova timer resets when Magtheridon stops casting it, which is needed to ensure that bots use cubes.
// However, Magtheridon's Blast Nova cooldown actually runs from when he starts casting it. This means that if a Blast Nova
// is not interrupted or takes too long to interrupt, the timer will be thrown off for the rest of the encounter.
// Correcting this issue is complicated and probably would need some rewriting--I have not done so and
// and view the current solution as sufficient since in TBC a missed Blast Nova would be a guaranteed wipe anyway.
bool MagtheridonManageTimersAndAssignmentsAction::Execute(Event /*event*/)
{
    Unit* magtheridon = FindMagtheridon(botAI, bot);
    if (!magtheridon)
    {
        LogMagtheridonDebug(botAI, bot, "manage_hold",
            "reason=no_magtheridon", nullptr, 8);
        return false;
    }

    const uint32 instanceId = magtheridon->GetMap()->GetInstanceId();
    const time_t now = time(nullptr);

    bool blastNovaActive = magtheridon->HasUnitState(UNIT_STATE_CASTING) &&
                           magtheridon->FindCurrentSpellBySpellId(SPELL_BLAST_NOVA);
    bool lastBlastNova = lastBlastNovaState[instanceId];

    if (lastBlastNova && !blastNovaActive)
        blastNovaTimer[instanceId] = now;

    lastBlastNovaState[instanceId] = blastNovaActive;
    static std::unordered_map<uint32, bool> lastShadowCageState;
    bool shadowCage = magtheridon->HasAura(SPELL_SHADOW_CAGE);
    if (!lastShadowCageState.count(instanceId) || lastShadowCageState[instanceId] != shadowCage ||
        (blastNovaActive != (magtheridon->HasUnitState(UNIT_STATE_CASTING) &&
                             magtheridon->FindCurrentSpellBySpellId(SPELL_BLAST_NOVA))))
    {
        LogMagtheridonDebug(botAI, bot, "manage_state",
            "shadow_cage=" + std::string(shadowCage ? "1" : "0") +
            " blast_nova=" + std::string(blastNovaActive ? "1" : "0"),
            magtheridon, 8);
        lastShadowCageState[instanceId] = shadowCage;
    }

    if (!shadowCage)
    {
        if (IsMechanicTrackerBot(botAI, bot, MAGTHERIDON_MAP_ID, nullptr))
        {
            spreadWaitTimer.try_emplace(instanceId, now);
            blastNovaTimer.try_emplace(instanceId, now);
            dpsWaitTimer.try_emplace(instanceId, now);
            LogMagtheridonDebug(botAI, bot, "manage_state",
                "reason=tracker_timers_initialized", magtheridon, 8);
        }
    }
    else
    {
        ObjectGuid guid = bot->GetGUID();
        MagtheridonSpreadRangedAction::initialPositions.erase(guid);
        MagtheridonSpreadRangedAction::hasReachedInitialPosition.erase(guid);
        botToCubeAssignment.erase(guid);

        if (IsMechanicTrackerBot(botAI, bot, MAGTHERIDON_MAP_ID, nullptr))
        {
            spreadWaitTimer.erase(instanceId);
            blastNovaTimer.erase(instanceId);
            dpsWaitTimer.erase(instanceId);
            LogMagtheridonDebug(botAI, bot, "manage_state",
                "reason=shadow_cage_active_timers_cleared", magtheridon, 8);
        }
    }

    return false;
}
