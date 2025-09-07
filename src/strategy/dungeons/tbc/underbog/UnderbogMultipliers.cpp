#include "UnderbogMultipliers.h"
#include "UnderbogActions.h"
#include "ChooseTargetActions.h"
#include "UnderbogTriggers.h"

float UnderbogMultiplier::GetValue(Action* action)
{
    // Following proven pattern of minimal multipliers to avoid priority violations
    // All priorities are set directly in strategy triggers
    return 1.0f;
}

float BlackStalkerAddMultiplier::GetValue(Action* action)
{
    // Block DpsAssist when Spore Striders are present - prevents boss/add oscillation
    if (botAI->IsHeal(bot)) { return 1.0f; }
    
    // Check for Spore Striders using same pattern as trigger
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    bool striderPresent = false;
    
    for (auto& target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (unit && unit->GetEntry() == NPC_SPORE_STRIDER)
        {
            striderPresent = true;
            break;
        }
    }
    
    if (striderPresent && dynamic_cast<DpsAssistAction*>(action))
    {
        return 0.0f; // Block DpsAssist when striders present
    }
    
    return 1.0f;
}