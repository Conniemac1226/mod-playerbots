#include "BloodFurnaceMultipliers.h"
#include "ChooseTargetActions.h"
#include "BloodFurnaceTriggers.h"
#include "BloodFurnaceActions.h"

float KelidanChannelerMultiplier::GetValue(Action* action)
{
    Player* bot = botAI->GetBot();
    if (!bot || botAI->IsHeal(bot))
        return 1.0f;

    Creature* channeler = bot->FindNearestCreature(NPC_SHADOWMOON_CHANNELER, 120.0f);
    if (channeler && channeler->IsAlive() && channeler->IsInCombat() &&
        dynamic_cast<DpsAssistAction*>(action))
    {
        return 0.0f; // Focus active channelers before resuming boss DPS.
    }

    return 1.0f;
}
