#include "BloodFurnaceMultipliers.h"
#include "ChooseTargetActions.h"
#include "BloodFurnaceTriggers.h"
#include "BloodFurnaceActions.h"

float KelidanChannelerMultiplier::GetValue(Action* action)
{
    // Block DpsAssist when Shadowmoon Channelers are present - prevents boss/add oscillation
    if (botAI->IsHeal(bot)) { return 1.0f; }
    
    // Detect channelers reliably by entry
    Player* bot = botAI->GetBot();
    bool channelerPresent = false;
    if (bot)
    {
        if (Creature* channeler = bot->FindNearestCreature(NPC_SHADOWMOON_CHANNELER, 120.0f))
            channelerPresent = channeler->IsAlive();
    }
    
    if (channelerPresent && dynamic_cast<DpsAssistAction*>(action))
    {
        return 0.0f; // Block DpsAssist when channelers present
    }
    
    return 1.0f;
}
