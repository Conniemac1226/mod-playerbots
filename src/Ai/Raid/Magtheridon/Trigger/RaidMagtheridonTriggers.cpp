#include "RaidMagtheridonTriggers.h"
#include "RaidMagtheridonHelpers.h"
#include "Playerbots.h"

using namespace MagtheridonHelpers;

bool MagtheridonAutoPullReadyTrigger::IsActive()
{
    return IsMagtheridonAutoPullReady(botAI, bot);
}

bool MagtheridonFirstThreeChannelersEngagedByMainTankTrigger::IsActive()
{
    Unit* magtheridon = FindMagtheridon(botAI, bot);

    return magtheridon && botAI->IsMainTank(bot) &&
           IsMagtheridonChannelerPhaseActive(botAI, bot) &&
           IsMagtheridonEncounterPullAllowed(botAI, bot) &&
           magtheridon->HasAura(SPELL_SHADOW_CAGE);
}

bool MagtheridonNWChannelerEngagedByFirstAssistTankTrigger::IsActive()
{
    Unit* magtheridon = FindMagtheridon(botAI, bot);
    Creature* channelerDiamond = GetChanneler(bot, NORTHWEST_CHANNELER);

    return magtheridon && botAI->IsAssistTankOfIndex(bot, 0) &&
           IsMagtheridonChannelerPhaseActive(botAI, bot) &&
           IsMagtheridonEncounterPullAllowed(botAI, bot) &&
           channelerDiamond;
}

bool MagtheridonNEChannelerEngagedBySecondAssistTankTrigger::IsActive()
{
    Unit* magtheridon = FindMagtheridon(botAI, bot);
    Creature* channelerTriangle = GetChanneler(bot, NORTHEAST_CHANNELER);

    return magtheridon && botAI->IsAssistTankOfIndex(bot, 1) &&
           IsMagtheridonChannelerPhaseActive(botAI, bot) &&
           IsMagtheridonEncounterPullAllowed(botAI, bot) &&
           channelerTriangle;
}

bool MagtheridonPullingWestAndEastChannelersTrigger::IsActive()
{
    Unit* magtheridon = FindMagtheridon(botAI, bot);

    Creature* channelerStar = GetChanneler(bot, WEST_CHANNELER);
    Creature* channelerCircle = GetChanneler(bot, EAST_CHANNELER);

    return magtheridon && bot->getClass() == CLASS_HUNTER &&
           IsMagtheridonChannelerPhaseActive(botAI, bot) &&
           IsMagtheridonEncounterPullAllowed(botAI, bot) &&
           (channelerStar || channelerCircle);
}

bool MagtheridonDeterminingKillOrderTrigger::IsActive()
{
    Unit* magtheridon = FindMagtheridon(botAI, bot);

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
        return false;

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
        return std::any_of(npcs.begin(), npcs.end(), [this](const ObjectGuid& npc)
        {
            Unit* unit = botAI->GetUnit(npc);
            return unit && unit->GetEntry() == NPC_BURNING_ABYSSAL;
        });
}

bool MagtheridonBossEngagedByMainTankTrigger::IsActive()
{
    Unit* magtheridon = FindMagtheridon(botAI, bot);

    return magtheridon && botAI->IsMainTank(bot) &&
           IsMagtheridonEncounterPullAllowed(botAI, bot) &&
           !magtheridon->HasAura(SPELL_SHADOW_CAGE);
}

bool MagtheridonBossEngagedByRangedTrigger::IsActive()
{
    Unit* magtheridon = FindMagtheridon(botAI, bot);

    return magtheridon && !magtheridon->HasAura(SPELL_SHADOW_CAGE) &&
           IsMagtheridonEncounterPullAllowed(botAI, bot) && botAI->IsRanged(bot);
}

bool MagtheridonIncomingBlastNovaTrigger::IsActive()
{
    Unit* magtheridon = FindMagtheridon(botAI, bot);
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
    }

    return botToCubeAssignment.find(bot->GetGUID()) != botToCubeAssignment.end();
}

bool MagtheridonNeedToManageTimersAndAssignmentsTrigger::IsActive()
{
    return FindMagtheridon(botAI, bot);
}
