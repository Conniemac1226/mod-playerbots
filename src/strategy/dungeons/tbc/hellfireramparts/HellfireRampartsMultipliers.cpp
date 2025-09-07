#include "HellfireRampartsMultipliers.h"
#include "ChooseTargetActions.h"
#include "HellfireRampartsTriggers.h"

float OmorAddMultiplier::GetValue(Action* action)
{
    // Block DpsAssist when Fiendish Hounds are present - prevents boss/add oscillation
    if (botAI->IsHeal(bot)) { return 1.0f; }
    
    // Check for Fiendish Hounds using same pattern as trigger
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    bool houndPresent = false;
    
    for (auto& target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (unit && unit->IsInCombat() && unit->GetEntry() == NPC_FIENDISH_HOUND)
        {
            houndPresent = true;
            break;
        }
    }
    
    if (houndPresent && dynamic_cast<DpsAssistAction*>(action))
    {
        return 0.0f; // Block DpsAssist when hounds present
    }
    
    return 1.0f;
}