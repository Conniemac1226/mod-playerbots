#include <unordered_map>
#include <ctime>

#include "Log.h"
#include "RaidMagtheridonMultipliers.h"
#include "RaidMagtheridonActions.h"
#include "RaidMagtheridonHelpers.h"
#include "ChooseTargetActions.h"
#include "GenericSpellActions.h"
#include "Playerbots.h"
#include "WarlockActions.h"
#include "WipeAction.h"

using namespace MagtheridonHelpers;

namespace
{
    std::string DescribeMagtheridonAction(Action* action)
    {
        if (!action)
            return "action=none";

        std::string type = "other";
        if (dynamic_cast<AttackAction*>(action))
            type = "attack";
        else if (dynamic_cast<CastSpellAction*>(action))
            type = "cast";
        else if (dynamic_cast<MovementAction*>(action))
            type = "move";
        else if (dynamic_cast<WipeAction*>(action))
            type = "wipe";

        return "action_name=" + action->getName() + " action_type=" + type;
    }
}

// Don't do anything other than clicking cubes when Magtheridon is casting Blast Nova
float MagtheridonUseManticronCubeMultiplier::GetValue(Action* action)
{
    Unit* magtheridon = FindMagtheridon(botAI, bot);
    if (!magtheridon)
        return 1.0f;

    if (magtheridon->HasUnitState(UNIT_STATE_CASTING) &&
        magtheridon->FindCurrentSpellBySpellId(SPELL_BLAST_NOVA))
    {
        auto it = botToCubeAssignment.find(bot->GetGUID());
        if (it != botToCubeAssignment.end())
        {
            if (dynamic_cast<WipeAction*>(action))
                return 1.0f;
            else if (!dynamic_cast<MagtheridonUseManticronCubeAction*>(action))
            {
                LogMagtheridonDebug(botAI, bot, "multiplier_cube_block",
                    DescribeMagtheridonAction(action) + " reason=blast_nova_active " +
                    GetMagtheridonTargetDecisionFields(botAI ? botAI->GetUnit(bot->GetTarget()) : nullptr, magtheridon, nullptr, "multiplier_cube_block", "non_cube_action"),
                    magtheridon, 5);
                return 0.0f;
            }
        }
    }

    return 1.0f;
}

// Bots will wait for 6 seconds after Magtheridon becomes attackable before engaging
float MagtheridonWaitToAttackMultiplier::GetValue(Action* action)
{
    Unit* magtheridon = FindMagtheridon(botAI, bot);
    if (!magtheridon || magtheridon->HasAura(SPELL_SHADOW_CAGE))
        return 1.0f;

    if (botAI->IsMainTank(bot))
        return 1.0f;

    const uint8 dpsWaitSeconds = 6;
    auto it = dpsWaitTimer.find(magtheridon->GetMap()->GetInstanceId());
    if (it == dpsWaitTimer.end() ||
        (time(nullptr) - it->second) < dpsWaitSeconds)
    {
        if (dynamic_cast<AttackAction*>(action) ||
            (!botAI->IsHeal(bot) && dynamic_cast<CastSpellAction*>(action)))
        {
            LogMagtheridonDebug(botAI, bot, "multiplier_wait_block",
                DescribeMagtheridonAction(action) + " reason=wait_to_attack " +
                GetMagtheridonTargetDecisionFields(botAI ? botAI->GetUnit(bot->GetTarget()) : nullptr, magtheridon, nullptr, "multiplier_wait_block", "attack_or_cast"),
                magtheridon, 5);
            return 0.0f;
        }
    }

    return 1.0f;
}

float MagtheridonDisableOffTankAssistMultiplier::GetValue(Action* action)
{
    Unit* magtheridon = FindMagtheridon(botAI, bot);
    if (!magtheridon || !IsMagtheridonChannelerPhaseActive(botAI, bot))
        return 1.0f;

    if (bot->GetVictim() == nullptr)
        return 1.0f;

    if ((botAI->IsAssistTankOfIndex(bot, 0) || botAI->IsAssistTankOfIndex(bot, 1)) &&
        dynamic_cast<TankAssistAction*>(action))
        return 0.0f;

    return 1.0f;
}

float MagtheridonChannelerTargetMultiplier::GetValue(Action* action)
{
    Unit* magtheridon = FindMagtheridon(botAI, bot);
    if (!magtheridon || botAI->IsHeal(bot) ||
        !IsMagtheridonChannelerPhaseActive(botAI, bot))
        return 1.0f;

    bool channelerAlive =
        GetChanneler(bot, SOUTH_CHANNELER) ||
        GetChanneler(bot, WEST_CHANNELER) ||
        GetChanneler(bot, EAST_CHANNELER) ||
        GetChanneler(bot, NORTHWEST_CHANNELER) ||
        GetChanneler(bot, NORTHEAST_CHANNELER);

    if (channelerAlive &&
        (dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<TankAssistAction*>(action)))
    {
        LogMagtheridonDebug(botAI, bot, "multiplier_channeler_block",
            DescribeMagtheridonAction(action) + " reason=channelers_alive " +
            GetMagtheridonTargetDecisionFields(botAI ? botAI->GetUnit(bot->GetTarget()) : nullptr, magtheridon, nullptr, "multiplier_channeler_block", "dps_or_tank_assist"),
            magtheridon, 5);
        return 0.0f;
    }

    return 1.0f;
}
