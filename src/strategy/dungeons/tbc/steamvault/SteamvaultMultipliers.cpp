#include "SteamvaultMultipliers.h"
#include "ChooseTargetActions.h"
#include "GenericSpellActions.h"
#include "SteamvaultTriggers.h"
#include "Unit.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

float SteamvaultMultiplier::GetValue(Action* action)
{
    return 1.0f;
}

float ThespiaWaterElementalMultiplier::GetValue(Action* action)
{
    if (!action)
        return 1.0f;

    Player* bot = botAI->GetBot();
    if (!bot || botAI->IsHeal(bot))
        return 1.0f;

    if (!HasAttackableThespiaWaterElemental(botAI, bot))
        return 1.0f;

    if (action->getName() == "attack water elemental")
    {
        return 3.0f;
    }

    if (dynamic_cast<DpsAssistAction*>(action) || action->getThreatType() == Action::ActionThreatType::Aoe)
        return 0.0f;

    return 1.0f;
}

float SteamriggerMechanicMultiplier::GetValue(Action* action)
{
    if (!action)
        return 1.0f;

    Player* bot = botAI->GetBot();
    if (!bot || botAI->IsHeal(bot))
        return 1.0f;

    if (!HasAttackableSteamriggerMechanic(botAI, bot))
        return 1.0f;

    if (dynamic_cast<DpsAssistAction*>(action) || action->getThreatType() == Action::ActionThreatType::Aoe)
        return 0.0f;

    return 1.0f;
}

float KalithreshSpellReflectionMultiplier::GetValue(Action* action)
{
    if (!action)
        return 1.0f;

    Player* bot = botAI->GetBot();
    if (!bot)
        return 1.0f;

    Unit* boss = AI_VALUE2(Unit*, "find target", "warlord kalithresh");
    if (!boss || !boss->IsAlive() || !boss->IsInCombat() || !boss->HasAura(SPELL_SPELL_REFLECTION))
        return 1.0f;

    CastSpellAction* spellAction = dynamic_cast<CastSpellAction*>(action);
    if (!spellAction)
        return 1.0f;

    return spellAction->GetTarget() == boss ? 0.0f : 1.0f;
}

float KalithreshDistillerMultiplier::GetValue(Action* action)
{
    if (!action)
        return 1.0f;

    Player* bot = botAI->GetBot();
    if (!bot || botAI->IsHeal(bot))
        return 1.0f;

    if (!HasAttackableKalithreshDistiller(botAI, bot))
        return 1.0f;

    if (dynamic_cast<DpsAssistAction*>(action) || action->getThreatType() == Action::ActionThreatType::Aoe)
        return 0.0f;

    return 1.0f;
}
