#include "BloodFurnaceMultipliers.h"
#include "ChooseTargetActions.h"
#include "BloodFurnaceTriggers.h"

float KelidanChannelerMultiplier::GetValue(Action* action)
{
    // Block DpsAssist when Shadowmoon Channelers are present - prevents boss/add oscillation
    if (botAI->IsHeal(bot)) { return 1.0f; }
    
    // Check for Shadowmoon Channelers using same pattern as trigger
    Unit* channeler = AI_VALUE2(Unit*, "find target", "shadowmoon channeler");
    bool channelerPresent = (channeler && channeler->IsAlive());
    
    if (channelerPresent && dynamic_cast<DpsAssistAction*>(action))
    {
        return 0.0f; // Block DpsAssist when channelers present
    }
    
    return 1.0f;
}