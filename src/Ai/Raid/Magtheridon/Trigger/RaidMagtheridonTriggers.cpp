#include "RaidMagtheridonTriggers.h"
#include "RaidMagtheridonHelpers.h"
#include "Playerbots.h"

using namespace MagtheridonHelpers;

bool MagtheridonAutoPullReadyTrigger::IsActive()
{
    bool active = IsMagtheridonAutoPullReady(botAI, bot);
    if (!active)
        LogMagtheridonDebug(botAI, bot, "trigger_autopull_inactive", GetMagtheridonEncounterDebug(botAI, bot), FindMagtheridon(botAI, bot), 10);
    return active;
}

bool MagtheridonFirstThreeChannelersEngagedByMainTankTrigger::IsActive()
{
    Unit* magtheridon = FindMagtheridon(botAI, bot);

    bool active = magtheridon && botAI->IsMainTank(bot) &&
           IsMagtheridonChannelerPhaseActive(botAI, bot) &&
           IsMagtheridonEncounterPullAllowed(botAI, bot) &&
           magtheridon->HasAura(SPELL_SHADOW_CAGE);
    if (active)
        LogMagtheridonDebug(botAI, bot, "trigger_channeler_main_tank", GetMagtheridonEncounterDebug(botAI, bot, magtheridon), magtheridon, 10);
    return active;
}

bool MagtheridonNWChannelerEngagedByFirstAssistTankTrigger::IsActive()
{
    Unit* magtheridon = FindMagtheridon(botAI, bot);
    Creature* channelerDiamond = GetChanneler(bot, NORTHWEST_CHANNELER);

    bool active = magtheridon && botAI->IsAssistTankOfIndex(bot, 0) &&
           IsMagtheridonChannelerPhaseActive(botAI, bot) &&
           IsMagtheridonEncounterPullAllowed(botAI, bot) &&
           channelerDiamond;
    if (active)
        LogMagtheridonDebug(botAI, bot, "trigger_channeler_assist0", GetMagtheridonEncounterDebug(botAI, bot, magtheridon), magtheridon, 10);
    return active;
}

bool MagtheridonNEChannelerEngagedBySecondAssistTankTrigger::IsActive()
{
    Unit* magtheridon = FindMagtheridon(botAI, bot);
    Creature* channelerTriangle = GetChanneler(bot, NORTHEAST_CHANNELER);

    bool active = magtheridon && botAI->IsAssistTankOfIndex(bot, 1) &&
           IsMagtheridonChannelerPhaseActive(botAI, bot) &&
           IsMagtheridonEncounterPullAllowed(botAI, bot) &&
           channelerTriangle;
    if (active)
        LogMagtheridonDebug(botAI, bot, "trigger_channeler_assist1", GetMagtheridonEncounterDebug(botAI, bot, magtheridon), magtheridon, 10);
    return active;
}

bool MagtheridonPullingWestAndEastChannelersTrigger::IsActive()
{
    Unit* magtheridon = FindMagtheridon(botAI, bot);

    Creature* channelerStar = GetChanneler(bot, WEST_CHANNELER);
    Creature* channelerCircle = GetChanneler(bot, EAST_CHANNELER);

    bool active = magtheridon && bot->getClass() == CLASS_HUNTER &&
           IsMagtheridonChannelerPhaseActive(botAI, bot) &&
           IsMagtheridonEncounterPullAllowed(botAI, bot) &&
           (channelerStar || channelerCircle);
    if (active)
        LogMagtheridonDebug(botAI, bot, "trigger_hunter_channelers", GetMagtheridonEncounterDebug(botAI, bot, magtheridon), magtheridon, 10);
    return active;
}

bool MagtheridonDeterminingKillOrderTrigger::IsActive()
{
    Unit* magtheridon = FindMagtheridon(botAI, bot);
    Unit* previousTarget = botAI ? botAI->GetUnit(bot->GetTarget()) : nullptr;

    Creature* channelerSquare   = GetChanneler(bot, SOUTH_CHANNELER);
    Creature* channelerStar     = GetChanneler(bot, WEST_CHANNELER);
    Creature* channelerCircle   = GetChanneler(bot, EAST_CHANNELER);
    Creature* channelerDiamond  = GetChanneler(bot, NORTHWEST_CHANNELER);
    Creature* channelerTriangle = GetChanneler(bot, NORTHEAST_CHANNELER);

    if (!magtheridon || botAI->IsHeal(bot) || botAI->IsMainTank(bot) ||
        !IsMagtheridonChannelerPhaseActive(botAI, bot) ||
        !IsMagtheridonEncounterPullAllowed(botAI, bot) ||
        (botAI->IsAssistTankOfIndex(bot, 0) && channelerDiamond) ||
        (botAI->IsAssistTankOfIndex(bot, 1) && channelerTriangle))
    {
        LogMagtheridonDebug(botAI, bot, "trigger_kill_order_inactive",
            GetMagtheridonTargetDecisionFields(previousTarget, magtheridon, nullptr, "trigger_kill_order_inactive", "trigger_kill_order_active") +
            " " + GetMagtheridonEncounterDebug(botAI, bot, magtheridon), magtheridon, 10);
        return false;
    }

    LogMagtheridonDebug(botAI, bot, "trigger_kill_order_active",
        GetMagtheridonTargetDecisionFields(previousTarget, magtheridon, nullptr, "trigger_kill_order_active", "trigger_kill_order_inactive") +
        " " + GetMagtheridonEncounterDebug(botAI, bot, magtheridon), magtheridon, 10);
    return channelerSquare || channelerStar || channelerCircle ||
           channelerDiamond || channelerTriangle ||
           (magtheridon && !magtheridon->HasAura(SPELL_SHADOW_CAGE));
}

bool MagtheridonBurningAbyssalSpawnedTrigger::IsActive()
{
    Unit* magtheridon = FindMagtheridon(botAI, bot);
    if (!magtheridon || bot->getClass() != CLASS_WARLOCK)
        return false;

    const GuidVector& npcs = AI_VALUE(GuidVector, "nearest hostile npcs");
    bool active = std::any_of(npcs.begin(), npcs.end(), [this](const ObjectGuid& npc)
        {
            Unit* unit = botAI->GetUnit(npc);
            return unit && unit->GetEntry() == NPC_BURNING_ABYSSAL;
        });
    if (active)
        LogMagtheridonDebug(botAI, bot, "trigger_abyssal_active", GetMagtheridonEncounterDebug(botAI, bot, magtheridon), magtheridon, 10);
    return active;
}

bool MagtheridonBossEngagedByMainTankTrigger::IsActive()
{
    Unit* magtheridon = FindMagtheridon(botAI, bot);

    bool active = magtheridon && botAI->IsMainTank(bot) &&
           IsMagtheridonEncounterPullAllowed(botAI, bot) &&
           !magtheridon->HasAura(SPELL_SHADOW_CAGE);
    if (active)
        LogMagtheridonDebug(botAI, bot, "trigger_boss_main_tank", GetMagtheridonEncounterDebug(botAI, bot, magtheridon), magtheridon, 10);
    return active;
}

bool MagtheridonBossEngagedByRangedTrigger::IsActive()
{
    Unit* magtheridon = FindMagtheridon(botAI, bot);
    Unit* previousTarget = botAI ? botAI->GetUnit(bot->GetTarget()) : nullptr;

    bool active = magtheridon && !magtheridon->HasAura(SPELL_SHADOW_CAGE) &&
           IsMagtheridonEncounterPullAllowed(botAI, bot) && botAI->IsRanged(bot);
    if (active)
        LogMagtheridonDebug(botAI, bot, "trigger_boss_ranged",
            GetMagtheridonTargetDecisionFields(previousTarget, magtheridon, nullptr, "trigger_boss_ranged", "trigger_kill_order_inactive") +
            " " + GetMagtheridonEncounterDebug(botAI, bot, magtheridon), magtheridon, 10);
    return active;
}

bool MagtheridonIncomingBlastNovaTrigger::IsActive()
{
    Unit* magtheridon = FindMagtheridon(botAI, bot);
    Unit* previousTarget = botAI ? botAI->GetUnit(bot->GetTarget()) : nullptr;
    Group* group = bot->GetGroup();
    if (!group || !magtheridon || magtheridon->HasAura(SPELL_SHADOW_CAGE))
        return false;

    bool needsReassign = botToCubeAssignment.empty();
    if (!needsReassign)
    {
        for (auto const& pair : botToCubeAssignment)
        {
            Player* assigned = ObjectAccessor::FindPlayer(pair.first);
            if (!assigned || !assigned->IsAlive())
            {
                needsReassign = true;
                break;
            }
        }
    }

    if (needsReassign)
    {
        std::vector<CubeInfo> cubes = GetAllCubeInfosByDbGuids(bot->GetMap(), MANTICRON_CUBE_DB_GUIDS);
        AssignBotsToCubesByGuidAndCoords(group, cubes, botAI);
        LogMagtheridonDebug(botAI, bot, "trigger_cube_reassign",
            "assigned=" + std::to_string(botToCubeAssignment.count(bot->GetGUID())) +
            " cubes=" + std::to_string(cubes.size()),
            magtheridon, 10);
    }

    bool active = botToCubeAssignment.find(bot->GetGUID()) != botToCubeAssignment.end();
    if (active)
        LogMagtheridonDebug(botAI, bot, "trigger_cube_active",
            GetMagtheridonTargetDecisionFields(previousTarget, magtheridon, nullptr, "trigger_cube_active", "dps_priority_attack") +
            " " + GetMagtheridonEncounterDebug(botAI, bot, magtheridon), magtheridon, 10);
    return active;
}

bool MagtheridonNeedToManageTimersAndAssignmentsTrigger::IsActive()
{
    Unit* magtheridon = FindMagtheridon(botAI, bot);
    bool active = magtheridon != nullptr;
    if (active)
        LogMagtheridonDebug(botAI, bot, "trigger_manage_timers",
            GetMagtheridonTargetDecisionFields(botAI ? botAI->GetUnit(bot->GetTarget()) : nullptr, magtheridon, nullptr, "trigger_manage_timers", "none") +
            " " + GetMagtheridonEncounterDebug(botAI, bot, magtheridon), magtheridon, 10);
    return active;
}
